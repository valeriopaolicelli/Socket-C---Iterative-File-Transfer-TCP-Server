        /**********************************************************************************/
 	   /**HEADER FOR CLIENT'S HANDLER			                                         */ 
      /***Purpose: library for declaration of functions and structure for client entity */
     /****Author: Valerio Paolicelli - Matricola: 253054                           	   */
    /*****Assignment of course "Distribuited Programming I"            			      */
   /******Master Degree in Computer Engineering - Data Science                       */
  /*******Politecnico di Torino - 21/05/2018                                        */
 /********                                                                         */
/**********************************************************************************/

#ifndef CLIENT_H_INCLUDED

#define CLIENT_H_INCLUDED

typedef struct client *CLIENT;

#define BUFLEN 				1024 // buffer length 
#define N_SECONDS 	  		  60 // for timeout
#define MAX_ERR       		  50 // arbitrary max value of errors 
#define LENGHT_MSG 	  		  13 // lenght of response message, with +OK size and dimension of requested file
#define LENGHT_QUIT_MSG 	   6 // lenght of termination message
#define LENGHT_RESPONDE_MSG    5

CLIENT init_client(char *addr, char *port); // function to initialize the structure containing the variables 
								            // which build a client entity (address, port number etc.)
int connect_client(CLIENT c); // function to perform the connection of client (creation of socket, connection with server etc.)
int send_request_client(CLIENT c, char *file); // function to send GET request to server
int send_quit_client(CLIENT c); // function to send quit message to server
int receive_file_client(CLIENT c, char *file); // function to receive the requested file
void free_client(CLIENT c); // function to release structures

#endif