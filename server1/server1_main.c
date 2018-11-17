        /**********************************************************************************************************/
       /**SEQUENTIAL SERVER MAIN                                                                                 */
      /***Purpose: general handler of server operations from initialization of server/connection to close of it */
     /****Author: Valerio Paolicelli - Matricola: 253054                                                       */
    /*****Assignment of course "Distribuited Programming I"                                                   */
   /******Master Degree in Computer Engineering - Data Science                                               */
  /*******Politecnico di Torino - 21/05/2018                                                                */
 /********                                                                                                 */
/**********************************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "../sterr.h"
#include "../myserver.h"
#include "../errlib.h"
#include "../sockwrap.h"
/* GLOBAL VARIABLES */
SERVER s;
ST st;
char *prog_name;

// Signal handler, when ctrl+C occurs in the server (SIGINT) or any client dies (SIGPIPE) or client ends, the program: 
//                                              - prints a message;
//                                              - kills itself releasing all resources only in case of SIGINT
//                                              - connection, between server and all its connected clients, is closed
//                                              - remains still in waiting for other clients in case of SIGPIPE
void sig_manager(int signo){
  int index;
  //in case of SIGINT, the server prints a message, performes the deallocation of all structures and dies.
  if(signo== SIGINT){
    fprintf(stdout,"\n***********************Abort - Connection closed!***********************\n");
    close_socket_server(s); // close socket with failed client
    
    //deallocation
    free_server(s);
    free_st(st);

    exit(0); //close server program - release all resources
  }

  //in case of client failure, the server prints a message of broke pipe, and still wait other clients (it doesn't terminate)
  if(signo== SIGPIPE){
     index= SIG_PIPE_CODE;
     print_err_msg(st, index); //print error message with function of ST errors library
     //set a field (flag) of the symbole table structure  equal to a special value, which indicate that the server close the connection
     //with current client (causing by broken pipe), but remains in listening for others.
     set_prev_msg(st, index);
     //close_socket_server(s); // close socket with failed client
  } 
}

int main(int argc, char *argv[]){ 
  // program receives one parameter equal to the listen port number

  // every operation is followed by check of function result
  // in case of error, every resources is released and deallocated and the program terminate with message error and code
  
  // for connection and accept, the errors are handled in the sockwrap
  int index; // variable used to check the return value of functions, which may computes a termination with error code
  signal(SIGINT,sig_manager); // intercept signal SIGINT like in case of ctrl+c
  signal(SIGPIPE,sig_manager); // intercept signal SIGPIPE like in case of irregular closing of connection from client side
                               // for example in case of timeout expiration
  prog_name = argv[0];
  if (argc != 2) {
    printf("Usage: %s <port number>\n", prog_name);
    exit(1);
  }
  s= init_server(argv[1]); // initialize the structure of server variables
  st= init_st(MAX_ERR); // initialize the structure of symbol table of errors 
  connect_server(s); // perform the connection (Creation of socket + Bind + Listen)

  // this server serves the clients in sequential mode
  // it is a server, this means that it doesn't terminate its execution normally, but only with special comman, like ctrl+c, 
  // otherwise it loops forever

  for (;;){
    // server loops forever accepting the requests of incoming clients, 
    // enstablish the comunication (socket) with the new client, serve the GET request and close the link
    accept_client_server(s); 
    index= send_response_server(s); // serves client requests
    check_index(st, index); // checks returned code with function of ST errors - in case of error it prints a message, 
                            // so remains in waiting for new requests
    close_socket_server(s);  // at the end of current client, close active socket and serve the others in queue (or wait new one)
  }    
  return 0;
}