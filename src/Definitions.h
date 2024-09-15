/* Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ANSI-color-codes.h" // Credit to RabaDabaDoba: https://gist.github.com/RabaDabaDoba

/* Definitions */
#define MAX_BUFFER 1024                        // max line buffer
#define MAX_ARGS 64                            // max # args
#define SEPARATORS " \t\n"                     // token separators
#define CURRENT_DIRECTORY_SIZE 200             // Size associated with current directory buffer

/* Function Prototypes, divided into (Startup, Execution, Internal, I/O Redirection, Batch mode, Background exec, Misc.) */
void setShellEnv(char** argv); // Startup

void handler(char** args, int argc); // Execution
int internalExecution(char** args, int argc, int* input_fd_pointer, int* output_fd_pointer);
void externalExecution(char** args, int argc, int* input_fd_pointer, int* output_fd_pointer);

void internalCDs(char** args, int argc); // Internal
int checkForInternal(char* command);
void cd(char** args, int argc);
void dir(char** args, int argc);
void runEnviron();
void clr();
void quit();
void echo(char** args, int argc);
void help();
void pauseShell();

void parseArgs(char** args, int argc, int* input_fd_pointer, int* output_fd_pointer); // I/O Redirection
void setFDs(char mode, char* fileName, int* input_fd_pointer, int* output_fd_pointer);
void redirectIO(int inputFD, int outputFD);
void cleanup(int inputFD, int outputFD);

FILE* batchCheck(int argc, char** argv, int* flagPointer); // Batch mode

int backgroundCheck(char** args, int argc); // Background exec

void clearBuffer(char* buffer, int size); // Misc.
int countArgs(char** args);
void outputPrompt(char* prompt);

/* Variables */
extern char** environ;
extern int errno;