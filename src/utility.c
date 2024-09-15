#include "Definitions.h"

/* ######VARIABLES###### */
pid_t pid;                                      // Store process pid
int input_fd;                                   // Hold input file descriptor
int output_fd;                                  // Hold output file descriptor
char** arg;                                     // Working args pointer
char currentDirectory[CURRENT_DIRECTORY_SIZE];  // Buffer to store result from cwd()

/* ######SHELL STARTUP###### */

// To be run at program start, sets the SHELL enviroment variable to the directory in which the program was executed in
// argv[0] will be used to supply the program's name
void setShellEnv(char** argv) {
    int len = strlen(argv[0]);
    char name[len]; // Used to hold prog name without '.' at the start
    memset(name, 0, len); // Used to set every element in name to 0 (prevent garbage values)

    getcwd(currentDirectory, CURRENT_DIRECTORY_SIZE);
    for (int i = 1; i < len; i++) { // i = 1 to ignore '.' at the start of argv[0]
        name[i - 1] = argv[0][i];
    }
    name[len] = '\0'; // set null terminator

    strcat(currentDirectory, name);
    setenv("SHELL", currentDirectory, 1);
}

/* ######EXECUTION HANDLING###### */

// Handles input from main
// Responsible for shell function after command has been entered
void handler(char**args, int argc) {
    // Set input_fd and output_fd to default file descriptors (stdin and stdout)
    input_fd = STDIN_FILENO;
    output_fd = STDOUT_FILENO;

    // Check for Internal Commands
    arg = args;
    if (internalExecution(arg, argc, &input_fd, &output_fd)) {
        return;
    }

    /* else pass command onto OS */
    externalExecution(arg, argc, &input_fd, &output_fd);
    return;   
}

// Handles internal execution, 0 is returned early if no interal command is found
int internalExecution(char** args, int argc, int* input_fd_pointer, int* output_fd_pointer) {
    // Check to see if internal command is being executed
    if (!checkForInternal(args[0])) {
        return 0;
    }

    // Special case required for quit
    // Required here as fork() is being used, using quit() in the child process will only terminate the child not the parent
    if (!strcmp(args[0], "quit")) {
        quit();
    }

    // Similar case as quit
    // CD'ing in a child process won't change the directory the parent is in
    if (!strcmp(args[0], "cd")) {
        cd(args, argc);
        return 1;
    }

    // Requires the pid of the parent in order to function
    // Which is why it's here
    if (!strcmp(args[0], "help")) {
        help();
    } 

    // Parse the input command, looking for I/O redirection characters (<, >, >>)
    parseArgs(args, argc, input_fd_pointer, output_fd_pointer);

    // Fork then redirect if necessary
    switch (pid = fork()) {
        case -1:
            fprintf(stdout, "Wallahi this fork did not work (internal)\n");
            exit(-1);
            break;
        case 0:
            redirectIO(*input_fd_pointer, *output_fd_pointer);
            internalCDs(args, argc);
            exit(0);
            break;
        default:
            if (backgroundCheck(args, argc)) {
                fprintf(stdout, "Process executing in background. Process ID: %d\n.", pid);
            }
            else {
                waitpid(pid, NULL, 0);
            }
            break;
            
    }

    // Close any file streams associated with I/O redirection
    cleanup(*input_fd_pointer, *output_fd_pointer);
    
    // Return 1 to return back to main
    return 1;
}

// Responsible for handling programs that are not internal
// Looks for programs to execute in "/bin"
void externalExecution(char** args, int argc, int* input_fd_pointer, int* output_fd_pointer) {
    // Get absolute path to program
    char* progname = (char*) calloc(sizeof(char), CURRENT_DIRECTORY_SIZE);

    if (!progname) { // Error check
        perror("Memory Allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    strcat(progname, "/bin/");
    strcat(progname, args[0]);

    // Test to see if program exists and is executable
    if (access(progname, F_OK | X_OK)) {
        fprintf(stdout, "%s: command not found\n", args[0]);
        return;
    }

    // Parse the input command, looking for I/O redirection characters (<, >, >>)
    parseArgs(args, argc, input_fd_pointer, output_fd_pointer);

    // Check for background execution
    int backgroundFlag = backgroundCheck(args, argc); // Check here as execv() replaces process image

    // Generate command line args for the program
    int commandLen = 0;
    while(args[commandLen]) {commandLen++;};

    if (backgroundFlag) {  // To not include "&" in commandArgs
        commandLen--;
    }

    char* commandArgs[commandLen + 1];
    for (int i = 0; i < commandLen; i++) {
        commandArgs[i] = args[i];
    }
    commandArgs[commandLen] = '\0'; // execv() requires that the array of pointers is null terminated

    // Fork and exec the program, redirect if necessary
    // PARENT environment variable should be set here too
    switch (pid = fork())
    {
    case -1:
        fprintf(stdout, "%sWallahi this fork did not work (external)%s\n", RED, reset);
        abort();
        break;
    case 0:
        setenv("PARENT", getenv("SHELL"), 1); // Set PARENT environment variable when child is made
        redirectIO(*input_fd_pointer, *output_fd_pointer);
        execv(progname, commandArgs);
        fprintf(stdout, "%sError occured with program execution.%s\n", RED, reset); // Output message if execv() fails
        break;
    default:
        if (backgroundFlag) {
            fprintf(stdout, "Process executing in background. Process ID: %d\n.", pid);
        }
        else {
            waitpid(pid, NULL, 0);
        }
        break;
    }

    // Close any file streams associated with I/O redirection
    cleanup(*input_fd_pointer, *output_fd_pointer);
    free(progname); // Free memory allocated
    progname = NULL;
}

/* ######INTERNAL COMMANDS###### */

// Executes given internal command based on name
// Excludes "quit", "cd", "help" for reasons mentioned above, see line: 56
void internalCDs(char** args, int argc) {
    if (!strcmp(args[0], "clr")) {
        clr();
    }

    if (!strcmp(args[0], "dir")) {
        dir(args, argc);
    }

    if (!strcmp(args[0], "environ")) {
        runEnviron();
    }

    if (!strcmp(args[0], "echo")) {
        echo(args, argc);
    }

    if (!strcmp(args[0], "pause")) {
        pauseShell();
    }
}

// Checks if the command to be executed is internal or not
// Returns 1 if command is internal else 0
int checkForInternal(char* command) {
    char internalCommandList[10][10] = {
        "echo",
        "cd",
        "environ",
        "dir",
        "help",
        "clr",
        "pause",
        "quit",
    };

    for(int i = 0; i < 10; i++) {
        if (!strcmp(command, internalCommandList[i])) {
            return 1;
        }
    }

    return 0;
}

// Used to change the current working directory
void cd(char** args, int argc) {
    if (args[1] == NULL) { // Check to see if dir was supplied
        fprintf(stdout,"%s\n",currentDirectory);
    }
    else {
        strcat(currentDirectory, "/");
        if (!chdir(args[1])) { // Error check
            setenv("PWD", strcat(currentDirectory, args[1]), 1);
        }
        else {
            fprintf(stdout, "%sDirectory does not exist.%s\n", RED, reset);
        }
    }
}

// Clears the screen of previous output
void clr() {
    system("clear");
}

// Lists the contents of a given directory (current by default)
void dir(char** args, int argc) {
    char cmd[100];
    strcpy(cmd, "ls -al ");

    if (args[1]) {
        strcat(cmd, args[1]);
    }
    system(cmd);
}

// Outputs all environment variables in the format NAME=VALUE
void runEnviron() {
    for(char** tmpEnviron = environ; *tmpEnviron; tmpEnviron++) { // goes until NULL is reached
        fprintf(stdout, "%s\n", *tmpEnviron);
    }
}

// Outputs args to screen
void echo(char** args, int argc) {
    for (int i = 1; args[i]; i++) { // goes until NULL is reached
        fprintf(stdout, "%s ", args[i]);
    }
    fputs("\n", stdout);
}

// Reason why this is so long as this needs to be executable from anywhere
// Takes advantage of sym links to find the path of the process's executable (path of myshell)
// Function is very roundabout, most likely a better way of doing this
void help() {
    char buffer[150];               // Working buffer
    char pidHolder[10];             // Used to hold the process's pid in str form
    char tmpCurrentdir[150];        // Holds the current working directory before any work happens
    FILE* fp;                       // Working file pointer
    int size;                       // Used to hold buffer's size at one point

    memset(buffer, '\0', 150); // Sets all elements in buffer to NULL, to avoid junk
    
    sprintf(pidHolder, "%d", getpid()); // Get's the process's pid and converts it to str form

    getcwd(tmpCurrentdir, 150); // Holds the current working directory to cd to later after everything is finished

    // This set of commands find's the executable path and stores it into a text file (currentProcessExecutablePath.txt)
    strcat(buffer, "readlink -f /proc/");
    strcat(buffer, pidHolder);
    strcat(buffer, "/exe > currentProcessExecutablePath.txt");
    system(buffer);

    // Responsible for retrieving the path from currentProcessExecutablePath.txt
    memset(buffer, '\0', 150); // Sets all elements in buffer to NULL, remove any previous contents
    fp = fopen("currentProcessExecutablePath.txt", "r");
    fgets(buffer, 150, fp);
    size = strlen(buffer);
    remove("currentProcessExecutablePath.txt");

    // Removes the executable name form the path
    for (int i = size - 1; buffer[i] != '/'; i--) {
        buffer[i] = '\0';
    }

    // Changes dir to the executable path, then outputs manual to the screen
    // After everything is done change back to original dir
    chdir(buffer);
    system("more ../manual/readme.md");
    chdir(tmpCurrentdir);
    
    // Cleanup
    fclose(fp);
    fp = NULL;
}

// Pauses shell activity until Enter is pressed
void pauseShell() {
    fprintf(stdout, "Press Enter to continue | ");
    while (getc(stdin) != '\n') {}
}

// Exits the shell
void quit() {
    exit(0);
}

/* ######I/O REDIRECTION###### */

// Parses an input string to look for redirection characters (<, >, >>), if found then input_fd and output_fd are set accordingly
void parseArgs(char** args, int argc, int* input_fd_pointer, int* output_fd_pointer) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp(args[i], "<")) {
            setFDs('<', args[i + 1], input_fd_pointer, output_fd_pointer);
            args[i] = NULL; // args set to null to limit args supplied to a program
        }

        else if (!strcmp(args[i], ">")) {
            setFDs('>', args[i + 1], input_fd_pointer, output_fd_pointer);
            args[i] = NULL;
        }

        else if (!strcmp(args[i], ">>")) {
            setFDs('3', args[i + 1], input_fd_pointer, output_fd_pointer);
            args[i] = NULL;
        }
    }
}

// Sets input_fd/output_fd to the file descriptors of files to be redirected to 
void setFDs(char mode, char* fileName, int* input_fd_pointer, int* output_fd_pointer) {    
    if (fileName) { // Check to see if a file name was supplied
        switch (mode) {
            case '<':
                if (!access(fileName, F_OK)) { // Check to see if file exists
                   *(input_fd_pointer) = fileno(fopen(fileName, "r"));
                }
                else {
                    fprintf(stdout, "%sFile doesn't exist.%s\n", RED, reset);
                }
                
                break;

            case '>':
                *(output_fd_pointer) = fileno(fopen(fileName, "w"));
                break;

            case '3':
                *(output_fd_pointer) = fileno(fopen(fileName, "a"));
                break;
        }
    }
}

// Performs I/O redirection using file descriptors and dup2() (ONLY USE IN CHILD PROCESS)
void redirectIO(int inputFD, int outputFD) {
    dup2(input_fd, STDIN_FILENO);
    dup2(output_fd, STDOUT_FILENO);
}

// Closes any files that were used in I/O redirection
void cleanup(int inputFD, int outputFD) {
    if (input_fd != STDIN_FILENO) {
        close(input_fd);
    }
    
    if (output_fd != STDOUT_FILENO) {
        close(output_fd);
    }
}

/* ######BATCH MODE###### */

// Checks whether an additional argument is supplied at shell start, if it is verify file exists then open the file
// Returns FILE* stream if file is supplied and exists, else returns stdin
FILE* batchCheck(int argc, char** argv, int* flagPointer) {
    if (argc == 2 && !access(argv[1], F_OK)) {
        *flagPointer = 1;
        return fopen(argv[1], "r");
    }

    return stdin;
}

/* ######BACKGROUND EXECUTION###### */

// Checks to see if "&" is the last argument, if it is return 1 else 0
int backgroundCheck(char** args, int argc) {
    if (!strcmp(args[argc - 1], "&")) {
        return 1;
    }

    return 0;
}

/* ######HELPER FUNCTIONS###### */

// Clears a given buffer (currentDirectory)
void clearBuffer(char * buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = 0;
    }
}

// Counts the number of args in a given line from args
int countArgs(char** args) {
    int i = 0;
    while(args[i]) {i++;}
    return i;
}

// Refresh current directory buffer and output prompt + current directory to stdout
void outputPrompt(char* prompt) {
    clearBuffer(currentDirectory, CURRENT_DIRECTORY_SIZE);      // Used to refresh the buffer
    getcwd(currentDirectory, CURRENT_DIRECTORY_SIZE);           // Gets the current working directory and stores in the currentDirectory buffer

    // BGRN and reset reference text colours found in ANSI-color-codes.h
    fprintf(stdout, "%s%s%s %s%s%s %s%s%s",
    BCYN, prompt, reset, BHWHT, currentDirectory, reset, BCYN, "|| ", reset);
}