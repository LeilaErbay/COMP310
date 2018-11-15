// Author: Leila Erbay
// ID: 260672158
// Purpose: Source code for all the different tiny shell instances


// all libraries that need to be accessed
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <wordexp.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <linux/sched.h>
#include <signal.h>
#include <ctype.h>

#include "funcDef.h"            // my header file containing of this program

#define MILLION 1000000         //constant value for time conversion
#define STACK_SIZE  8192        //constant value for stack size in clone 



char * fifoFile;        //global var for FIFO file
int fifoInt;            //global var for FIFO operation (write vs read)
FILE *reportFile;       //global var for collecting timing and sending it to a file



/* function: isInputValid
    Purpose: checks if the input is valid (doesn't start with new line tab, or its length is 0)
             this function is called inside all of the system implementations and called inside exec
    Input: char * (ie user command line input)
    Return:  int
            -1 if invalid user entry
            0 if valid user entry
*/
int isInputValid(char * input){
    int ret = 0;
    if(input[0] == '\0' || input[0] == '\n' || input[0] == '\t' || strlen(input) <= 1){
        ret = -1;
    }
    return ret;
} 



/*  function: readIn
    Purpose: Read in command from keyboard
    Input: char* (user input)
    Return: Nothing
*/
void readIn(char *input){
    fgets (input, 512, stdin);
}



/*  Name: exec
    Purpose: parse the string into separate words 
             apply parsed string to execvp command to execute user command
             for clone if user enters cd then it allows user to change directors
    Input: char * (user input)
    Return: Int
            returns the status of execvp if the function executes execvp else it exits with an error

    Note: Errors if the user enters less than 1 char* (checked by isInputValid and condition)
          if user enters cd in CLONE process then it brings user to the home directory

*/
int exec(char *input){
    wordexp_t p;     
    char **w;

    if (isInputValid(input)== -1) exit(EXIT_FAILURE);   //if some kind of wrong input is entered then exit FAIL

    removeNewLine(input);           // removes \n character at end of user input

    wordexp(input, &p, 0);          //parses user input appropriately
    
    w = p.we_wordv;         //w points to the vector of strings

    if (p.we_wordc <1) exit(EXIT_FAILURE);      //if no words exist then fail


    //FOR CLONE: if user enters cd
    if(strcmp(w[0], "cd") == 0){  
                                  //if user enters cd
        if (w[1] == NULL ){
            w[1] = getenv("HOME");
            cd(w[1]);

        }
        else if(cd(w[1]) <0) {
            perror("Error in cd");
        }

       
    }
    else{
        char** cmds = {w, NULL};    //cmds : contains all commands of user and NULL
        int stat = execvp(w[0], cmds);         //executes cmds similar to OS "system"
        //if (stat == -1) printf("fail");
        return stat;
    }
        return 0;
   // wordfree(&p);
}



/*
    Name: RemoveNewLine
    Purpose: removes the \n char at the end of the user's input and replaces it with \0
             so that execvp can work properly
    Input: char * (user input)
    Return: nothing
*/
void removeNewLine(char* input){
    if (input[strlen(input)-1] == '\n') {    //remove newline and replace with null terminator
        input[strlen(input)-1] = '\0';
    }
}

/* Name: my_system
   Purpose: executes system, fork, vfork, clone, depending on flag
   Input: char * (user command)
   Return: return value from the specified version
 */
int my_system(char * input){
    #ifdef FORK
        return mySystemFork(input);

    #elif VFORK
        return mySystemVFork(input);
    
    #elif CLONE
        
        return mySystemClone(input);

    
    #elif PIPE
        return mySystemFifo(input);
    
    #else
        return mySystemUnix(input);

    #endif

    return 0;


}

/*  Name: report
    Purpose: write to report file the timing of the particular version of the tiny shell for each command
    Input: long startTime - start time of the running command
            char* systemApp - the particular version of the tiny version
            char*input - the user input string
    Return: Nothing

    Note: Updates Global Var reportFile (FILE *)

*/
void report(long startTime,char* systemApp, char* input){
    removeNewLine(input);
    long endTime = getTime();
    long duration = endTime - startTime;    //determine duration

    reportFile = fopen("reportFile.txt", "a"); 
    fprintf(reportFile, "%s : %s -- %ld ms\n", systemApp, input, duration);     //prints to reportFile
    fclose(reportFile);

}


/*  Name: mySystemUnix
    Purpose: Call the Unix system() to execute user input
    Input: char* (user input)
    Return: int
            if 0 then continue process
            if -3 then user entered "exit"

    Note: enters data into reportFile on timing

*/
int mySystemUnix(char* string){
    long startTime = getTime();

    int stat  = 0;
    if (strncmp(string, "exit",4) != 0){        //call system if user did not enter 
        system(string);
    }
    else if(strncmp(string, "exit", 4) == 0){
        stat = -3;
    }

    report(startTime, "System", string);
    
    return stat;
}




/*  Name: mySystemFork
    Purpose: executes tiny Shell with fork
    Input: char* (user input)
    Return: Int 
            if -3 then user entered "exit"
            if -2 then error occurred somewhere

    Note: Alters reportFile with data on timing
*/
int mySystemFork(char *string){
    int ret;        
    pid_t pid;
    int stat;

    long startTime = getTime();         // start timing

        if(strncmp(string, "exit", 4) != 0){        //enter condition if user hasn't entered exit
            
            if ((pid = fork()) < 0) {       //if pid from fork < 0 then error occurred
                perror("Fork error");
                exit(EXIT_FAILURE);
            }

            //  In  CHILD - execute command line
            else if (pid == 0){                     //if pid == 0 then continue execution
                if(isInputValid(string) != -1){     //as long as input is valid
                    int ret = exec(string); 
                    if (ret == -1) exit(errno);     // if execvp failed -- get error
                    else exit(EXIT_SUCCESS);        //else exit from child successfully
                }
                
            }  

            //In PARENT
            else{
               int waitStat =  waitpid(pid, &stat, 0);  //get status of child 

                if( waitStat == -1 ) {
                    perror("error occurred in waitpid");        //error in waitpid
                    ret = -2;
                }
                else if( WIFEXITED(stat) && WEXITSTATUS(stat) != 0 ) {
                    printf("child failed with exit status = %d\n",WEXITSTATUS(stat));           /* The child failed! */
                    ret = -2;
                }
                else ret =0 ;

            }
        }

        else ret = -3; //user entered exit

        report(startTime,"Fork", string);       //determine timing

        return ret;
}




/*  Name: mySystemVFork
    Purpose: use vfork to implement tiny shell
    Input: char * (user input)
    Return: int
            return of the process 
            -2 if error occurred
            -3 if user entered "exit"

    Note: Changes reportFile with data on timing

*/

int mySystemVFork(char *string){
    int ret;            //return value of the function
    pid_t pid;
    int stat;       //status of child process

    long startTime = getTime();


        if(strncmp(string, "exit", 4) != 0){        //if user does not enter exit
            
            if ((pid = vfork()) < 0) {      
                perror("Fork error");           //fork error
                exit(EXIT_FAILURE);
            }

            //  In  CHILD - execute command line
            else if (pid == 0){
                if(isInputValid(string) != -1){
                    int ret = exec(string); 
                    if (ret == -1) exit(errno);     // if execvp failed -- get error
                    else exit(EXIT_SUCCESS);        //else exit from child successfully
                }
                
            }  

            //In PARENT
            else{
               int waitStat =  waitpid(pid, &stat, 0);

                if( waitStat == -1 ) {
                    perror("error occurred in waitpid");        //error in waitpid
                    ret = -2;
                }
                else if( WIFEXITED(stat) && WEXITSTATUS(stat) != 0 ) {
                    printf("child failed with exit status = %d\n",WEXITSTATUS(stat));           /* The child failed! */
                    ret = -2;
                }
                else ret = 0;
            }
        }

        else ret = -3;      //user entered "exit"

       report(startTime, "VFork", string);  //enter timing report
        return ret;
}




/*  Name: mySystemClone
    Purpose: implement tiny shell using clone 
    Input: char * (user input)
    Return: int
            return of the process 
            -2 if error occurred
            -3 if user entered "exit"

    Note: Timing is printed to stdout, report file can't be edited
*/
int mySystemClone(char * input){
    
    int ret;
 

    long startTime = getTime();
    
    char cwd[250];
    getcwd(cwd, sizeof(cwd));

    void* childStack = malloc(STACK_SIZE);                  //stack for child
    void* stackTop = childStack + 8192;
    int flags = CLONE_VFORK | CLONE_FS | CLONE_FILES ;       //flags for child

    if(strncmp(input, "exit", 4) != 0){                             // if user has not entered exit
        int cpid = clone( &childFxn, stackTop, flags | SIGCHLD , input);        //call to childFxn
        

        int stat;
        int waitStat =  wait(&stat);                //wait for child


         if( waitStat == -1 ) {
            perror("error occurred in waitpid");        //error in waitpid
            ret =-2;
        }
        else if( WIFEXITED(stat) && WEXITSTATUS(stat) != 0 ) {
            printf("child failed with exit status = %d\n",WEXITSTATUS(stat));           /* The child failed! */
            ret = -2;
        }
        else ret = 0;
    }
    else ret = -3;      //user entered exit
    

    //report(startTime, "Clone", input);  //determine timing

    removeNewLine(input);
    long endTime = getTime();
    long duration = endTime - startTime;    //determine duration

    printf("%s : %s -- %ld ms\n", "Clone", input, duration);     //prints to reportFile
  

    return ret;
}



/*  Name: childFxn
    Purpose: Executes the input entered by the user
    Input: void *arg (user input is passed)
    Return: Int or exit error or success

*/
int childFxn(void *arg){
        
    if (getpid()  < 0) {
        perror("clone error");
        exit(EXIT_FAILURE);
    }
        //  In  CHILD - execute command line
    else if (getpid() > 0){
        if(isInputValid(arg) != -1){
            int ret = exec(arg); 
            if (ret == -1) exit(errno);     // if execvp failed -- get error
            else exit(EXIT_SUCCESS);        //else exit from child successfully
        }
                
    }   
    return 0;
}



/*  Name: cd
    Purpose: allow to change directory
    Name: char * (user input)
    Return: the value of chdir
*/
int cd(char *input){
   return chdir(input);
}



/*  Name: setGlobalVars
    Purpose: Set global variables for pipe file and fifo setting
    Input: char * fileName, int fifoSetting
    Output: void

    Note: It alters global variables fifoFile and fifoInt
*/
void setGlobalVars(char * fileName, int fifoSetting){
    fifoFile = fileName;
    fifoInt = fifoSetting;
   
}



/*  Name: createFifo
    Purpose: make fifo connection if needed
    Input: None
    Outout: None

    Note: exit failure if mkfifo fails 
*/
void createFifo(){
    int fiforet = mkfifo(fifoFile, 0666);
    if (fiforet == -1) {
     perror("mkfifo error");    //error if mkfifo fails
     exit(EXIT_FAILURE);
    }
}




/*  Name: mySystemFifo
    Purpose: use fifo for communication between two processes
    Input: char * (user input)
    Output: int
            return of the process 
            -2 if error occurred
            -3 if user entered "exit"
*/
int mySystemFifo(char * input){
    int ret;
    int stat;
    pid_t pid;

    //fifoFile and fifoInt are impt vars

    int saved_stdin = dup(0);
    int saved_stdout = dup(1);
  

    if(strncmp(input, "exit", 4) != 0) {        //if user doesn't enter exit
        if((pid= fork()) < 0) {
            perror("fork error");       //error if forking error occurred
            exit(EXIT_FAILURE);
        }
        else if(pid == 0) {             //no forking error
            
            //WRITING TO FIFO
            if (fifoInt == 1) {             
                    
                if(isInputValid(input) != -1){              
                    
                    int fdIn = open(fifoFile, O_WRONLY);    //writing
                
                    if (fdIn < 0 ) {
                        perror("child error: open for write error");    //error if opening failed
                        exit(EXIT_FAILURE);
                    }
                    else{
                        dup2(fdIn,1); // direct output to file
                        exec(input);
                        close(fdIn);

                        dup2(saved_stdout,1);
                        dup2(saved_stdin,0);
                    }                    
                }
            }   
            //READING TO FIFO
            if (fifoInt == 0){

                 if(isInputValid(input) != -1 ) {
                    wordexp_t p;     
                    char **w;

                    char buffer[512];               // buffer for reading
                    int fdOut = open(fifoFile, O_RDONLY);
                    
                    if (fdOut < 0 ) {
                        perror("child error: open for read error");
                        exit(EXIT_FAILURE);
                    }
                    else{
                        dup2(fdOut, 0);    //read from file
                        removeNewLine(input);  // prepare for execvp

                        wordexp(input, &p, 0);
    
                        w = p.we_wordv;  

                       // char* fileVal = buffer; 
                        char** cmds = {w, buffer,NULL};    //cmds : contains all commands of user and NULL
                        int stat = execvp(w[0], cmds);  

                        close(fdOut);

                        dup2(saved_stdout,1);
                        dup2(saved_stdin,0);

                    }
            }  
            //else exit(EXIT_FAILURE);
        }
    }   
        //PARENT 
        else{
            int waitStat =  waitpid(pid, &stat, 0);
            if( waitStat == -1 ) {
                perror("error occurred in waitpid");        //error in waitpid
                ret = -2;
            }
            else if( WIFEXITED(stat) && WEXITSTATUS(stat) != 0 ) {
                printf("child failed with exit status = %d\n",WEXITSTATUS(stat));            
                //The child failed! 
                ret = -2;
            }
            else ret = 0;
        }
    }
    else ret = -3;  //if user enters exit
    
    return ret;
}



/*  Name: getTime
    Purpose: get the time of each command
    Input: None
    Output: long (time in microseconds)
*/
long getTime(){
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((tp.tv_sec * (long)MILLION) + (tp.tv_nsec*0.001));
}


