        /**********************************************************************************/
 	   /**HEADER FOR SERVER'S HANDLER			                                         */ 
      /***Purpose: library for declaration of functions and structure for server entity */
     /****Author: Valerio Paolicelli - Matricola: 253054                           	   */
    /*****Assignment of course "Distribuited Programming I"            			      */
   /******Master Degree in Computer Engineering - Data Science                       */
  /*******Politecnico di Torino - 21/05/2018                                        */
 /********                                                                         */
/**********************************************************************************/

#ifndef SERVER_H_INCLUDED

#define SERVER_H_INCLUDED

typedef struct server *SERVER; 

#define BUFLEN     		 1024 // buffer length 
#define N_SECONDS 		   60 // for timeout
#define MAX_ERR 		   50 // arbitrary max value of errors 
#define LENGHT_MSG 		   13 // lenght of response message, with +OK size and dimension of requested file
#define LENGHT_QUIT_WORD    6 // quit message is composed by 6 bytes: QUIT\r\n
#define LENGHT_ERR_MSG      6 // err message is composed by 6 bytes: -ERR\r\n
#define LENGHT_KEY_WORD     4 // key word is the message of 4 bytes: GET_

SERVER init_server(char *port); // function to initialize the structure containing the variables 
								// which build a server entity (like active/passive socket, address, port number etc.)
void connect_server(SERVER ser); // function to perform the connection of server (Bind + Listen)
void accept_client_server(SERVER ser); // function to accept the connection by a client
int send_response_server(SERVER ser); // function to send message to client
void close_socket_server(SERVER ser); // closes active socket with client
void close_passive_socket_server(SERVER ser); // closes passive socket
void free_server(SERVER ser); // releases all structures  

#endif