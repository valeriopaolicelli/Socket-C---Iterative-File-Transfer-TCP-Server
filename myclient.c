        /***********************************************************************/
       /**SOURCE CODE FOR CLIENT'S HANDLER                                    */ 
      /***Purpose: structure and functions of client entity (files receiver) */
     /****Author: Valerio Paolicelli - Matricola: 253054                    */
    /*****Assignment of course "Distribuited Programming I"                */
   /******Master Degree in Computer Engineering - Data Science            */
  /*******Politecnico di Torino - 21/05/2018                             */
 /********                                                              */
/***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "myclient.h"
#include "sockwrap.h"

struct client{
	int s;	// connected socket
	uint16_t tport_n; // server port number
	struct sockaddr_in saddr;
	struct in_addr	sIPaddr; 	// server IP addr. structure
	struct sockaddr_in 	from;
	int resultIP;
};

CLIENT init_client(char *addr, char *port){ // Initialization of client structure with all its fields
  uint16_t tport_h;
  char ip[100]; // string to store the translated address from host name to IP form
  int no_conn= 0; // flag used to check if the connection is possible
  CLIENT c= (CLIENT) malloc(sizeof(struct client));
  c->s= 0; // arbitrary
	c->resultIP = inet_aton(addr, &(c->sIPaddr));
	if (!(c->resultIP)){ // inet_aton failed, try to convert the address to ip form
                       // the previous fail could be caused by inserting of host name (for example localhost or  www.google.com,
                       // instead ip address
		if(hostname_to_ip(addr, ip)!=1){ // try to solve the host name
			printf("**** %s resolved to %s, try to connect...\n" , addr, ip);
			c->resultIP = inet_aton(ip, &(c->sIPaddr)); // try to convert again the address
			if (!(c->resultIP)){ // if failed again -> no connection is possible with this address
                             // probably wrong string is passed by command line
				no_conn= 1;
			}
		}
		else{
			no_conn= 1;
		}
    }
	if (sscanf(port, "%" SCNu16, &tport_h)!=1){ // store the port number used by server (check all done)
		no_conn= 1;
  }
  c->tport_n = htons(tport_h); // translate to network form
  if(no_conn== 1)
    return NULL; // return empty item
  return c; // return success item
}

int connect_client(CLIENT c){
	c->s= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // create socket
	if(c->s<0)
		return -5; // return error code
	else
	 	printf("*** Socket number: %d\n",c->s);

	bzero(&(c->saddr), sizeof(c->saddr));
	c->saddr.sin_family= AF_INET;
	c->saddr.sin_port= c->tport_n;
	c->saddr.sin_addr= c->sIPaddr;
 	showAddr("*** Connecting to target address", &(c->saddr));
  
  // non-blocking connect (sockwrap.c), capable to find unreachable address
  // there is a timeout: expired this, the connect produces error code
  if(connect_nonb(c->s, (struct sockaddr *) &(c->saddr), sizeof(c->saddr), N_SECONDS)< 0) 
    return -6; // return error code
  return 0; // return success code
}

int send_request_client(CLIENT c, char *file){
  char bufS[BUFLEN];
  sprintf(bufS,"GET %s\r\n",file); // for each new file received, it's produced a GET request
  printf("\n%s", bufS);
	if(sendto(c->s, bufS, strlen(bufS), 0, (struct sockaddr *) &(c->saddr), sizeof(c->saddr))<0) // send GET request to server
		return -9; // return error code - send had problems
	return 0; // return success code
}

int send_quit_client(CLIENT c){
	int n;
	char bufS[BUFLEN];
	strcpy(bufS,"QUIT\r\n"); // when all files are received, the QUIT message is sended
	n= sendn(c->s, bufS, LENGHT_QUIT_MSG, 0);
	if( n<=0 || n!= LENGHT_QUIT_MSG)
  		return -9; // return error code - send had problems
  	return 0; // return success code
}

int receive_file_client(CLIENT c, char *file){
	fd_set rset; // used to set a timeout
	struct timeval tval; // used to set a timeout
	FILE *f;
	char bufr[LENGHT_MSG]; // buffer to receive the messages from server
  char content_file[BUFLEN]; // buffer to receive the bytes of files from server
  int n; // number of bytes received or sended
  int i, j, lenght_file_name;
  uint32_t size_file, timestamp; // information retrived about file
  char *file_name;
	printf("waiting for response...\n");

  /************Waits the response of server until timeout expires*************/
	FD_ZERO(&rset);
	FD_SET(c->s, &rset);
	tval.tv_sec = N_SECONDS;
	tval.tv_usec = 0;
	n = Select(FD_SETSIZE, &rset, NULL, NULL, &tval);
 	if (n>0){
      n= recv(c->s,bufr,LENGHT_RESPONDE_MSG,0); // store into the buffer the first 5 bytes
                              // they should be +OK\r\n or -ERR\r (throw error code in case of different message)
      if (n!=-1 && n== LENGHT_RESPONDE_MSG){
          bufr[n] = '\0';            
          //printf("bufr: %s - n= %d\n", bufr, n);
      }
      else
          return -26; // return error code of wrong reading
  	}
  	else
      return -11; // return error code
  	if(strncmp(bufr,"-ERR\r",LENGHT_RESPONDE_MSG)==0){ // checks if is received -ERR\r (case such as file not found or wrong request)
    	memset(bufr,0,LENGHT_MSG);
   		n= recv(c->s,bufr,1,0); // receives last byte (\n)
   		if(n!= 1)
   			return -26; // error code of wrong reading
    	if(bufr[0]!='\n') // checks that last byte is LF
    	  return -22;
   		return -10; // return success code -> -ERR is the normal behaviour
 	}
    else if(strncmp(bufr,"+OK\r\n",LENGHT_RESPONDE_MSG)==0){ // checks if is received +OK
		lenght_file_name= strlen(file); // retrieve the file name, without the path -> in fact the file will be stored into the client workfolder
		for(i=lenght_file_name-1; i>= 0; i--){ // checks if the file is in one other directory instead in the local one
		  if(file[i]=='/'){
			break;
		  }
		}
		if(i>=0){ // / founded and is at i-th position
		  i++; // starts from the first character of file name
		  lenght_file_name= lenght_file_name-i+1; // update the value of variable refers to the lenght of file name
		  file_name= (char *) malloc(lenght_file_name*sizeof(char));
		  for(j= 0; j<lenght_file_name; j++){ // copy only the file name
			file_name[j]= file[j+i];
		  }
		}
		else
		  file_name= strdup(file);
		f= fopen(file_name,"wb"); // create the new file, which will contain the content transfer by server
		if(f==NULL)
			return -2; // return error code 	
		n= recv(c->s,&size_file,sizeof(uint32_t),0); // receives the size
		if(n<=0 || n!=sizeof(uint32_t))
			  return -3; // return error code
		n= recv(c->s,&timestamp,sizeof(uint32_t),0); // receives the timestamp
		if(n<=0 || n!=sizeof(uint32_t))
			return -4; // return error code
		size_file= ntohl(size_file);
		printf("\t\tFile: %s\n", file_name);
			printf("\t\tSize: %u\n", size_file);
			printf("\t\tLast modificatione: %u\n", ntohl(timestamp));
		memset(bufr,0,BUFLEN);

		if(size_file== 0) // receives empty file too
			fclose(f);
		else if(size_file>0){ // receives empty file too
			for(;;){ // until it receive all the content of file, equal to size_file
		  // timeout is setted, if the client doesn't receive nothing for some seconds (N_SECONDS) from the server, 
		  // it prints an error message and stop its execution
				FD_ZERO(&rset);
				FD_SET(c->s, &rset);
				tval.tv_sec = N_SECONDS;
				tval.tv_usec = 0;
				n = Select(FD_SETSIZE, &rset, NULL, NULL, &tval);
				if(n> 0){
					n= recv(c->s,content_file,BUFLEN,0); // store the received bytes of current file into content_file
					if(n<=0){ // in case of error, the file not completed is removed and an error message is printed
						fclose(f);
						remove(file_name);
						free(file_name);
						return -12; // return error code
					}
					size_file-= n;
					if(fwrite(content_file,1,n,f)!= n){ // fwrite error
						fclose(f);
						remove(file_name);
						free(file_name);
						return -27; // return error code
					}  
					//printf("size_file= %d - n= %d\n", size_file, n);
					if (size_file <= 0){ // all file is received
						memset(content_file,0,BUFLEN); // clear the buffer for content of file
						fclose(f);
						free(file_name);
						break; // *******************exit -> pass to next file or quit
					}
				}
				else{
					fclose(f);
					remove(file_name);
					free(file_name);
					return -11; // return error code -> timeout expired
				}
			}
		}
		else{
		  free(file_name);
			fclose(f);
		}
  }
  else
   	return -22; // return error code -> unknown message (it isn't +OK... or -ERR)
  return 0; // return success code
}

void free_client(CLIENT c){ // closes the socket and releases the structure
	close(c->s);
	free(c);
}