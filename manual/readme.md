# a-Shell Manual

## Introduction
Welcome to the a-Shell manual!

This manual provides an overview of its various features.

## Using a-Shell
To use a-Shell type in the name of the program + any additonal arguments you want to give it.

USAGE
    *program arg1 arg2*

## Internal Commands
a-Shell supports the following internal commands:

- `cd`: 
    DESCRIPTION
        Change the current working directory.
    USAGE
        cd arg1

- `clr`:
    DESCRIPTION
        Clears the screen of any previous output.
    USAGE
        clr

- `environ`:
    DESCRIPTION
        Outputs all variables in the current environment.
    USAGE
        environ

- `dir`:
    DESCRIPTION
        Prints details of a given directory (current by default).
    USAGE
        dir arg1

- `echo`:
    DESCRIPTION
        Display a line of text.
    USAGE
        echo arg1 arg2 arg3

- `help`:
    DESCRIPTION
        Prints the manual to the screen.
    USAGE
        help

- `pause`:
    DESCRIPTION
        Pauses the shell until the *Enter* key is pressed.
    USAGE
        pause

- `quit`: 
    DESCRIPTION
        Terminate the a-Shell session.
    USAGE
        quit

## External Commands
a-Shell can also execute external commands.
To execute an external command, simply type its name followed by any required arguments.

USAGE
    *externalCommand arg1 arg2*

EXAMPLE
    *gcc helloWorld.c -o helloWorld*

## I/O Redirection
a-Shell supports I/O redirection.

USAGE
    *program arg1 arg2 < inputFile > outputFile*

OPTIONS
    - `>`: Redirects standard output to a file, overwriting its contents. If file doesn't exist then it's created.
    - `>>`: Redirects standard output to a file, appending to its contents. If file doesn't exist then it's created.
    - `<`: Redirects standard input from a file.

## Batch mode execution
Instead of prompting a user to input commands, a-Shell can read commands from a file instead.
File is provided at shell start.
a-Shell will process all the commands from a file then exit.

USAGE
    *./a-Shell batchfile*

## Processes & Background Execution
a-Shell can execute processes in the background as follows:

- `Background Execution`:
    DESCRIPTION
        Executes a given command in the background
    USAGE
        Include '&' at the end of input
    EXAMPLE
        *program arg1 arg2 &*