        /********************************************************************/
       /**SOURCE CODE FOR SERVER'S HANDLER                                 */ 
      /***Purpose: structure and functions of server entity (files sender)*/
     /****Author: Valerio Paolicelli - Matricola: 253054                 */
    /*****Assignment of course "Distribuited Programming I"             */
   /******Master Degree in Computer Engineering - Data Science         */
  /*******Politecnico di Torino - 21/05/2018                          */
 /********                                                           */
/********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "myserver.h"
#include "sockwrap.h"
#include <sys/stat.h>
#include <time.h>

int send_file_server(SERVER ser, FILE *f); // function to send requested file to client

struct server{
  socklen_t addrlen;
  struct sockaddr_in  saddr, caddr;   /* server and client addresses */ 
  uint16_t lport_n;   /* port used by server (net/host ord.) */
  int bklog;      /* listen backlog */
  int conn_request_skt;   /* passive socket */
  int s;          /* connected socket */
  char abs_path_serv[BUFLEN*10];
};

SERVER init_server(char *port){
  uint16_t lport_h;
  SERVER s= (SERVER) malloc(sizeof(struct server));
  if (sscanf(port, "%" SCNu16, &lport_h)!=1) // stores into lport_h the received port
    return NULL; // return empty item -> error case
  s->lport_n = htons(lport_h); // convert the port number in network format used by server
  s->bklog = 4;
  realpath("./",s->abs_path_serv); // saves the absolute path of server workfolder, 
  								   // to compare with the path that could arrived by the client
  return s;  // return success item
}

void connect_server(SERVER ser){ // creation of socket + bind + listen
  ser->s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create socket
  printf("-> Socket number: %u\n",ser->s); 

    /* bind the socket to any local IP address */
  bzero(&(ser->saddr), sizeof(ser->saddr));
  ser->saddr.sin_family      = AF_INET;
  ser->saddr.sin_port        = ser->lport_n;
  ser->saddr.sin_addr.s_addr = INADDR_ANY;
  showAddr("*** Binding to address", &(ser->saddr));
  Bind(ser->s, (struct sockaddr *) &(ser->saddr), sizeof(ser->saddr));
  printf("*** Done.\n");

    /* listen */
  printf ("*** Listening at socket %d with backlog = %d \n",ser->s,ser->bklog);
  Listen(ser->s, ser->bklog);
  printf("*** Done.\n");

  ser->conn_request_skt = ser->s;
}

void accept_client_server(SERVER ser){ // accept incoming connection
  ser->addrlen = sizeof(struct sockaddr_in);
  ser->s = Accept(ser->conn_request_skt, (struct sockaddr *) &(ser->caddr), &(ser->addrlen));

  showAddr("*** Accepted connection from", &(ser->caddr)); // print the address and port of new connection
  printf("*** New socket: %u\n",ser->s); // print the number of new socket
}

int send_response_server(SERVER ser){ // after receiving a message from client, start the response of server:
									  // first of all there are several checks about received request,
								      // (well formed message, right request of protocol etc.) 
									  // and return an error code, in case of anomalies;
								      // instead if is all right, the server replies with a message:
								      // based on TCP protocol, it's sended +OK size and timestamp (13Byte) of founded file, 
									  // or -ERR in case of error (followed by closing of socket)

  char buf[BUFLEN]; // reception buffer 
  char string_to_send[LENGHT_MSG]; // buffer for string +OK...
  int i= 0, j, n, lenght_file_name;
  uint32_t n_byte, time; // used to store the result of stat function in unsigned integer (32bits) format
  char nameFile[BUFLEN], path[BUFLEN], abs_path_file[BUFLEN]; 
  char *file_name;
  char special_char[3]; // used to store the last two characters of received message, to check if they are CR and LF
  fd_set rset; // used to set a timeout
  struct timeval tval; // used to set a timeout
  FILE *f; 
  struct stat fileStat; // Used to obtain information about file, like timestamp and size 
  do{ // until server receives QUIT\r\n......

  	// timeout is setted, if the server doesn't receive nothing for some seconds (N_SECONDS) from the client, 
  	// it sends an error message and close the socket
    FD_ZERO(&rset);
    FD_SET(ser->s, &rset);
    tval.tv_sec = N_SECONDS;
    tval.tv_usec = 0;
    n = Select(FD_SETSIZE, &rset, NULL, NULL, &tval);
    if (n>0){
      n= recv(ser->s,buf,BUFLEN-1,0);
      if (n!=-1){ // if somethings is retrieved
          for(i=4; i<BUFLEN-1 && buf[i]!='\n'; i++); // find the end of message (if it is lower than the size of buffer) rappresented by LF
          buf[i+1]= '\0';  // sets immediatly after the string terminator
          special_char[0]= buf[i-1]; // store the last two characters of message, this maybe CR
          special_char[1]= buf[i]; // while this LF
      }
      else
          return -23; // return error code
    }
    else
      return -11; // return error code - timeout expired
    
    if(strncmp(buf,"QUIT",LENGHT_KEY_WORD)==0){ // if server receives QUIT to close the connection by client (TCP message terminator)
      if(strncmp(special_char,"\r\n",2)!=0){ // checks if is well formed message (QUIT\r\n)
        n= writen(ser->s,"-ERR\r\n",LENGHT_ERR_MSG);
        if( n<=0 || n!=LENGHT_ERR_MSG) 
          return -17; // return error code - write on socket had problems
        return -24; // return error code - QUIT message not well formed
      }
      else // received QUIT\r\n message, termination!
        break;
    }
    printf("\n--Received: %s",buf); // instead if there is another message -> show it and its sender
    showAddr("--from:", &(ser->caddr));
    sscanf(buf,"GET %s\r\n",nameFile);
    //printf("\tFile: %s\n",nameFile);
    if(strncmp(buf,"GET ",LENGHT_KEY_WORD)!=0){ // checks if it is a GET request
      n= writen(ser->s,"-ERR\r\n",LENGHT_ERR_MSG);
      if( n<=0 || n!=LENGHT_ERR_MSG)  // if there is a different command by GET 
      												 // -> server sends -ERR and closes the socket
        return -17; // return error code  - write on socket had problems
      return -8; // return error code -> different command by GET
    } 
    if(strncmp(special_char,"\r\n",2)!=0){ // checks if is well formed message (GET <filename>\r\n)
      n= writen(ser->s,"-ERR\r\n",LENGHT_ERR_MSG);
        if( n<=0 || n!=LENGHT_ERR_MSG) 
          return -17; // return error code - write on socket had problems
      return -25; // return error code
    }
    

    lenght_file_name= strlen(nameFile);
    for(i=lenght_file_name-1; i>= 0; i--){ // checks if the file is in one other directory instead in the local one
      if(nameFile[i]=='/'){
      	break;
      }
  	}
    if(i>=0){ // / founded and is at i-th position
    	for(j=0; j<=i; j++) // retrieves the path of the requested file
    		path[j]= nameFile[j];
    	realpath(path,abs_path_file); // trasforms the relative path in absolute
    	if(strcmp(abs_path_file, ser->abs_path_serv)!=0){ // checks if the path of the requested file is the same of the server workfolder
    		// in case of different path, server raise error -> FILE NOT FOUND
    		n= writen(ser->s,"-ERR\r\n",LENGHT_ERR_MSG);
        	if( n<=0 || n!=LENGHT_ERR_MSG) 
          		return -17; // return error code - write on socket had problems
      		return -2; // return error code
    	}
    	
    	i++; // starts from the first character of file name
    	lenght_file_name= lenght_file_name-i+1; // update the value of variable refers to the lenght of file name
		file_name= (char *) malloc(lenght_file_name*sizeof(char));
    	for(j= 0; j<lenght_file_name; j++) // copy only the file name
    		file_name[j]= nameFile[j+i];
    }
    else
    	file_name= strdup(nameFile);

    if((f= fopen(file_name,"rb")) ==NULL){ // try to open the requested file, 
    									  // in case of not found, server sends -ERR message and close the socket
      n= writen(ser->s,"-ERR\r\n",LENGHT_ERR_MSG);
      if( n<=0 || n!=LENGHT_ERR_MSG) 
        return -17; // return error code - write on socket had problems
      //printf("Error opening file %s\n", file_name);
      return -2; // return error code
    }
    if(stat(file_name,&fileStat) < 0){ // try to retrieve the information about the requested message
      n= writen(ser->s,"-ERR\r\n",LENGHT_ERR_MSG);
      if( n<=0 || n!=LENGHT_ERR_MSG) 
        return -17; // return error code - write on socket had problems
      return -15; // return error code
    }
    else{ // the received message is correct and the file is founded -> send to client +OK size timestamp
      time= htonl(fileStat.st_mtime);
      n_byte= htonl(fileStat.st_size);
      sprintf(string_to_send,"+OK\r\n"); 
      n= writen(ser->s, string_to_send, strlen(string_to_send));
      if( n<=0 || n!=strlen(string_to_send))
        return -16; // return error code - send on socket had problems
      n= sendn (ser->s, &n_byte, sizeof(uint32_t), 0);
      if( n<=0 || n!=sizeof(uint32_t))
        return -18; // return error code - send on socket had problems
      n= sendn (ser->s, &time, sizeof(uint32_t), 0);
      if( n<=0 || n!=sizeof(uint32_t))
        return -19; // return error code - send on socket had problems
      //printf("  Sended: %s %u %u\n", string_to_send, ntohl(n_byte), ntohl(time));
      send_file_server(ser, f); // function to transfer the requested file
      fclose(f);
      free(file_name);
    }
  }while(1);
  printf("--Received: %s", buf); // prints the last message received (QUIT\r\n) and the sender
  showAddr("--from:", &(ser->caddr));
  return 0; // return success code
}

int send_file_server(SERVER ser, FILE *f){ // receive the server item and the current requested (already opened) file
  char bufS[BUFLEN]; // buffer of server
  int i, n;
  while((i= fread(bufS,1,BUFLEN,f))){ // read 1MB from the file and stor them into buffer (bufS)
    if(i<0) // check fread error
      return -14; // return error code
    n= writen(ser->s, bufS, i);
    if( n<=0 || n!=i) // send the buffer and check the error case
      return -20; // return error code - send on socket had problems
  }
  return 0; // return success code
}

void close_socket_server(SERVER ser){ // closes active socket with client
  close(ser->s);
  ser->s= -1;
  return;
}

void close_passive_socket_server(SERVER ser){ // closes passive socket
  close(ser->conn_request_skt);
  ser->conn_request_skt= -1;
  return;
}

void free_server(SERVER ser){ // to "close" the server, firstly it's closed the passive socket and after is deallocated the structure
  close(ser->conn_request_skt);
  ser->conn_request_skt= -1;
  free(ser);
}