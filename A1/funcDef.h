#ifndef _funcDef_h
#define _funcDef_h



// function declarations so that all can be intertwined

//Functions to check user input and edit if necessary
int isInputValid(char * input);
void readIn(char *input);
void removeNewLine(char* input);

//function that calls execvp
int exec(char *input);

// different applications of tiny shell
int my_system(char * input);
int mySystemUnix(char* string);
int mySystemFork(char *string);
int mySystemVFork(char *string);
int mySystemClone(char * input);
int childFxn(void *arg);
int mySystemFifo(char * input);

// function to cd
int cd(char *input);


//helper functions related to FIFO
void setGlobalVars(char * fileName, int fifoSetting);
void createFifo();


// functions related to timing
long getTime();
void report(long startTime,char* systemApp, char* input);

#endif
