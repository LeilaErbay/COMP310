// Author: Leila Erbay
// ID : 260672158
// Purpose: Run different tiny shell implementations

/*Notes:

  COMPILING
  (1) Running unix system() requires no flag
  (2) Running fork tiny shell requires -FORK flag
  (3) Running vfork tiny shell requires -VFORK flag
  (4) Running clone tiny shell requires -CLONE flag
  (5) Running pipe tiny shell requires -PIPE flag
  (6) Compiling any version of tiny shell requires you to also include funcDef.c file
  
  TIMING
  (1) Timing for FORK, VFORK, CLONE tiny shells write to a file named "reportFile"


*/

#include "funcDef.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include <unistd.h>

#include <fcntl.h>
#include <sys/types.h>

#include <limits.h>


/*
Name: main
Args: at least 1  argument: compiled program name
      to run PIPE tiny shell, you must enter a file name and either
      1 -- write
      0 -- read
Purpose: Run specified version of tiny shell

Note: FIFO only created when user enters 1 <---- EDIT
*/
int main(int argc, char *argv[]){
   
   if (argc >1){
      if(atoi(argv[2]) != 0 && atoi(argv[2]) != 1){
        perror("Invalid input for FIFO processes. Enter 1 for write and 0 for read");
        exit(EXIT_FAILURE);
      } 
       setGlobalVars(argv[1], atoi(argv[2])); 
      /* if (atoi(argv[2])== 1){
            createFifo();
       }*/

      if (access(argv[1], F_OK) == -1 & atoi(argv[2]) == 1) {  /* check if fifo already exists if not then, create the fifo*/
         createFifo();
    }

   }

    while(1){
        printf("tshell> ");
        char cmdLine[512];
        readIn(cmdLine);    // reads in the user-entered set of commands
        int ret = my_system(cmdLine);   // my_system returns a value 
        if (ret == -3) exit(EXIT_SUCCESS); //if user has entered "exit" then break out of program
    } 
}   
