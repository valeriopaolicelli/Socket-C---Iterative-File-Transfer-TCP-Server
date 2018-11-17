        /**********************************************************************************************************/
       /**CONCURRENT SERVER MAIN                                                                                 */
      /***Purpose: general handler of server operations from initialization of server/connection to close of it */
     /****Author: Valerio Paolicelli - Matricola: 253054                                                       */
    /*****Assignment of course "Distribuited Programming I"                                                   */
   /******Master Degree in Computer Engineering - Data Science                                               */
  /*******Politecnico di Torino - 21/05/2018                                                                */
 /********                                                                                                 */
/**********************************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "../errlib.h"
#include "../sockwrap.h"
#include "../sterr.h"
#include "../myserver.h"
#include <sys/stat.h>
#include <time.h>
#include <wait.h>
#include <signal.h>
#include <unistd.h>
/* GLOBAL VARIABLES */
SERVER s;
ST st;
char *prog_name;

// Signal handler, when ctrl+C occurs in the server (SIGINT) or any client dies (SIGPIPE) or client ends, the program: 
//                                              - prints a message;
//                                              - kills itself releasing all resources only in case of SIGINT
//                                              - connection, between server and all its connected clients, is closed
//                                              - remains still in waiting for other clients in case of SIGPIPE
//                                              - kills child processes that are terminated their execution in case of SIGCHLD
void sig_manager(int signo){

  //in case of SIGINT, the server prints a message, performes the deallocation of all structures and dies.
  if(signo== SIGINT){
    close_socket_server(s); //close passive socket
    fprintf(stdout,"\n***********************Abort - Connection closed!***********************\n");

    //deallocation
    free_server(s);
    free_st(st);
    
    kill(0,SIGKILL); //close server program, it sends to itself kill signal, to die.
  }
  else if(signo== SIGCHLD){
    //kill zombie processes
    while(waitpid((pid_t)(-1), 0, WNOHANG)> 0){
    }
    fprintf(stdout,"\n***********************Abort zombie process!***********************\n");
  }

  //in case of client failure, the server prints a message of broke pipe, and still wait other clients (it doesn't terminate)
  else if(signo== SIGPIPE){
     print_err_msg(st,SIG_PIPE_CODE); //print error message with function of ST errors library
     //set a field (flag) of the symbole table structure  equal to a special value, which indicate that the server close the connection
     //with current client (causing by broken pipe), but remains in listening for others.
     set_prev_msg(st, SIG_PIPE_CODE);
  } 
}


int main(int argc, char *argv[]){
  // program receives one parameter equal to the listen port number

  // every operation is followed by check of function result
  // in case of error, every resources is released and deallocated and the program terminate with message error and code
  
  // for connect and accept, the errors are handled in the sockwrap
	int childpid; // to store the pid of child
  int index; // variable used to check the return value of functions, which may computes a termination with error code
  signal(SIGINT,sig_manager); // intercept signal SIGINT like in case of ctrl+c
	signal(SIGCHLD,sig_manager); // intercept signal SIGCHLD, which occurs when process dies, in this way the main process (server2_main) 
                               // should kill zombie process
  signal(SIGPIPE,sig_manager); // intercept signal SIGPIPE like in case of irregular closing of connection from client side
                               // for example in case of timeout expiration
  prog_name = argv[0];
  if (argc != 2){
    printf("Usage: %s <port number>\n", prog_name);
    exit(1);
  }
  s= init_server(argv[1]); // initialize the structure of server variables
  st= init_st(MAX_ERR); // initialize the structure of symbol table of errors
  connect_server(s); // perform the connection (Creation of socket + Bind + Listen)
 
  // this server serves the clients in concurrent mode
  // it is a server, this means that it doesn't terminate its execution normally, but only with special comman, like ctrl+c, 
  // otherwise it loops forever

  for (;;){
    accept_client_server(s);

    // each time client arrives, new processes is created, with fork, in this way the process father (server2_main) continue to loop waiting
    // other requests, while the child serves the new client arrived and dies at the end.
    if((childpid=fork())<0) {
    	fprintf(stderr, "fork() failed\n");
    	return -1;
    }
	  else if (childpid > 0) /* parent process */
	    close_socket_server(s); // only close the socket (this is handled by child process) and continue to wait othe client
	  else {/* child process */
	    close_passive_socket_server(s); // close the passive socket, that is handled by father process
	    index= send_response_server(s); // serve client request
      check_index(st, index); // checks returned code with function of ST errors - in case of error it prints a message
	    exit(0); // die when client is served
	  }
  }    
  return 0;
}