  	     /**************************************************************************/
 	    /**HEADER FOR SYMBOL TABLE OF ERROR CODE                                  */ 
       /***Purpose: library with declaration of functions and structure          */
      /****         to handle the error codes/message collect into symbol table */
     /*****Author: Valerio Paolicelli - Matricola: 253054               	   */
    /******Assignment of course "Distribuited Programming I"                  */
   /*******Master Degree in Computer Engineering - Data Science              */
  /********Politecnico di Torino - 21/05/2018                               */
 /*********                                                                */                                  
/**************************************************************************/

#ifndef STERR_H_INCLUDED

#define STERR_H_INCLUDED

#define _ERR_MSG
typedef struct symboltable *ST;

#define SIG_PIPE_CODE 20 // error code in case of broken pipe -> server use it to print massage, still remain in waiting of other clients

ST init_st(int maxN); // Initialize the symbol table structure
void print_err_msg(ST t, int index); // Direct access to ST -> Retrieves the error message and prints
int num_error(ST t); // Return the number of error messages considered 
int check_index(ST t, int index); // Check if returned code by function, is a error case -> Each returned code is negative ad greater than 1,
								  // compare to the position of message inside the ST
void set_prev_msg(ST t, int index); // Used to set a flag, that allow to print an error message without terminate the process (case of SIGPIPE/broken pipe)
void free_st(ST t); // Dealloc ST

#endif
