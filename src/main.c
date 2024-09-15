#include "Definitions.h"

int main (int argc, char ** argv)
{
    // Variables
    char buf[MAX_BUFFER];                           // line buffer
    char * args[MAX_ARGS];                          // pointers to arg strings
    char ** arg;                                    // working pointer thru args
    char* prompt;                                   // shell prompt
    FILE* input;                                    // Input buffer (either stdin or a file stream)
    int argCount;                                   // No. of strings in args
    int batchFlag;                                  // Check for batch mode execution

    // Set SHELL environment variable
    setShellEnv(argv);

    // Assigning Variables
    prompt = "<a-Shell> ||";
    batchFlag = 0;
    input = batchCheck(argc, argv, &batchFlag);

    /* keep reading input until "quit" command or eof of redirected input */
    while (!feof(input)) { /* get command line from input */
       
        // Outputs prompt + current directory
        // if batchFlag is 1 don't print
        if (!batchFlag) {
            outputPrompt(prompt);
        }


        if (fgets (buf, MAX_BUFFER, input)) { // read a line
            /* tokenize the input into args array */
            arg = args;
            *arg++ = strtok(buf,SEPARATORS);   // tokenise input

            while ((*arg++ = strtok(NULL,SEPARATORS)));
            argCount = countArgs(args); // Get argc

            // last entry will be NULL 
            if (args[0]) {                     // if there's anything there
                handler(args, argCount);
            }
        }
        else {
            exit(0);
        }

        // Set all previous args to nu;;
        for (int i = 0; i < argCount; i++) {
            args[i] = NULL;
        }
    }
    
    // If input isn't stdin then close
    if (batchFlag) {
        fclose(input);
    }

    return 0; 
}