        /******************************************************************************************************************/
       /**CLIENT MAIN                                                                                                    */
      /***Purpose: general handler of client operations from initialization of client/socket to close of the connection */
     /****Author: Valerio Paolicelli - Matricola: 253054                                                               */
    /*****Assignment of course "Distribuited Programming I"                                                           */
   /******Master Degree in Computer Engineering - Data Science                                                       */
  /*******Politecnico di Torino - 21/05/2018                                                                        */
 /********                                                                                                         */
/******************************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../errlib.h"
#include "../sterr.h"
#include "../sockwrap.h"
#include "../myclient.h"
#include <inttypes.h>
#include <ctype.h>
#include <unistd.h>

char *prog_name;

CLIENT c;
ST s;
void sig_manager(int signo){ 
  //Signal handler, when ctrl+C occurs in the client, the program: 
  //                                              - prints a message;
  //                                              - kills itself releasing all resources
  //                                              - this connection, between client and server, is closed
  if(signo== SIGINT){
    fprintf(stdout,"\n***********************Abort - Connection close!***********************\n");
    kill(0,SIGKILL); //kill itself
  }
}

int main(int argc, char **argv){
  // Program receives:
  // argv[1] -> ip server address
  // argv[2] -> port#
  // argv[3.....argc] -> name of files to perform a transfer (GET request)

  // every operation is followed by check of function result
  // in case of error, every resources is released and deallocated and the program terminate with message error and code
  int i;
  int index; // variable used to check the return value of functions, which may computes a termination with error code
  int n_files;
  char **files;
	if(argc<3){ //check number of parameters
    	printf("Error parameters\n");
    	return -1*(num_error(s)+1); // return error code
  }
  signal(SIGINT,sig_manager); // intercept signal SIGINT like in case of ctrl+c
  s= init_st(MAX_ERR); // initialize the structure of symbol table of error codes
  c= init_client(argv[1], argv[2]); // inizialize the structure of client variables
  if(c== NULL){ // checks returned content of client structure - in case of error (NULL value) it prints a message,
                // performes the deallocation of structures and dies 
  	index= 7;
   	print_err_msg(s, index-1);
	free_st(s);
   	return -index;	
   }
  index= connect_client(c); // try to link the client to the server at specified address and port number
  if(check_index(s, index)<0){ // checks returned code with function of ST errors - in case of error it prints a message, 
                               // performes the deallocation of structures and dies 
  	free_st(s);
    free_client(c); 
    return index;
  }

  n_files= argc-3; //store the number of files requested through the command line

  // All requested files are saved in a vector
  files= (char**) malloc(n_files*sizeof(char*));
  for(i=3;i<argc;i++){
   	files[i-3]= strdup(argv[i]);
  }

  // for each file, send a GET request to server and waiting for a response
  for(i=0;i<n_files;i++){
    // send GET request to server
  	index= send_request_client(c, files[i]); 
    if(check_index(s, index)<0){ // checks returned code with function of ST errors - in case of error it prints a message, 
                               // send the quit message to close the connection, performes the deallocation of structures and dies 
      send_quit_client(c);
      for(i=0;i<n_files;i++){
        free(files[i]);
      }
      free(files);
      free_st(s);
      free_client(c); 
      return index;
    }
    // receive response (file/error message) from server
	  index= receive_file_client(c, files[i]);
	  if(check_index(s, index)<0){ // checks returned code with function of ST errors - in case of error it prints a message, 
                               // send the quit message to close the connection, performes the deallocation of structures and dies 
		  send_quit_client(c);
      for(i=0;i<n_files;i++){
        free(files[i]);
      } 
      free(files);
		  free_st(s);
  		free_client(c); 
      return index;
	  }
  }

  // after all file request the client sends QUIT and exit
  index= send_quit_client(c);
  if(check_index(s, index)<0){ // checks returned code with function of ST errors - in case of error it prints a message, 
                               // performes the deallocation of structures and dies 
  	for(i=0;i<n_files;i++){
      free(files[i]);
    }
    free(files);
    free_st(s);
  	free_client(c); 
   	return index;
  }

  // Close connection and released every structures 
  printf("Connection close.\n"); 
  for(i=0;i<n_files;i++){
    free(files[i]);
  }
  free(files);
  free_st(s);
  free_client(c); 
  return 0;
}