/*
    UMBC CMSC 421
    Spring 2021
    Project 1

    Due Date: 2/28/21 11:59:00 pm

    Author Name: Muhammed Muktar
    Author email: mmuktar12@gmail.com
    Description: a simple linux shell designed to perform basic linux commands
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include "utils.h"

/*
    In this project, you are going to implement a number of functions to
    create a simple linux shell interface to perform basic linux commands.
    Please note that the below set of functions may be modified as you see fit.
    We are just giving the following as a suggestion. If you do use our
    suggested design, you *will* need to modify the function signatures (adding
    arguments and return types).
*/

/*
DEFINE FUNCTION PROTOTYPES (add arguments/return types as needed)
*/
void user_prompt_loop();
char *get_user_command();
char **parse_command(char *command);
void execute_command(char **parsedCommand, char* getCommand);


int main(int argc, char *argv[]) {
  /*
    Write the main function that checks the number of arguments passed to
    ensure no command-line arguments are passed; that is to say that if the
    number of arguments is greater than 1 then exit the shell with a message
    to stderr and return value of 1. Otherwise call the user_prompt_loop()
    function to get user input repeatedly until the user enters the "exit"
    command.
  */

    //argument is greater than one
    if (argc > 1){

        fprintf(stderr, "%s" , "Argument greater than one!");
        return 1;
  
    }
    //no arugment
    else {

        //runs prompt
        user_prompt_loop();
    
    }
  
    return 0;
}

/*
    user_prompt_loop():
    Get the user input using a loop until the user exits, prompting the user for
    a command. Gets command and sends it to a parser, then compares the first
    element to the two built-in commands ("proc", and "exit"). If it's none of
    the built-in commands, send it to the execute_command() function. If the
    user decides to exit, then exit 0 or exit with the user given value.
*/
void user_prompt_loop() {
    
    //boolean to exit loop
    int exitLoop = 0;
    
    //used to count parsed command size and check for a valid input
    int index;
    char *checkChar;

    //while user does not exit
    while (!exitLoop){

        printf("$ ");

        //gets command from user
        char *getCommand = get_user_command();

        //parse command for 2d c string
        char **parsedCommand;
        
        //makes sure getCommand is populated
        if (getCommand[0] != '\0') {
            
            //parses command
            parsedCommand = parse_command(getCommand);
       
            //checks for exit
            if (!strcmp(parsedCommand[0],"exit")) {

                //makes sure command is valid to use
                index = 0;
                checkChar = parsedCommand[index];

                //while not at end of parsedCommand
                while (checkChar != NULL) {

                    index++;
                    checkChar = parsedCommand[index];

                }

                //too many arguments for exit
                if (index >= 3) {

                    printf("Invalid use of exit. exit [optional integer]\n");

                }
                //checks if second arugment is an integer
                else if (index == 2 && !isdigit(*parsedCommand[1])){

                    printf("%s is not an integer\n", parsedCommand[1]);
                
                }
                else {
                
                    //only exit command given with no number
                    if (index == 1) {

                        //frees memory
                        free(getCommand);
                        free(parsedCommand);

                        exit(0);

                        //exits loop if exit fails
                        exitLoop = 1;

                    }
                    //number is given
                    else {

                        int getDigit = atoi(parsedCommand[1]);

                        //frees memory
                        free(getCommand);
                        free(parsedCommand);

                        exit(getDigit);

                        //exits loop if exit fails
                        exitLoop = 1;
                    }   
                
                }
            
            }
            else if (!strcmp(parsedCommand[0],"proc")) {
            
                //makes sure command is valid to use
                index = 0;
                checkChar = parsedCommand[index];

                //while not at end of parsedCommand
                while (checkChar != NULL) {

                    index++;
                    checkChar = parsedCommand[index];

                }

                //command would not be valid
                if (index != 2) {
                
                    printf("Invalid use of proc. proc [file path]\n");

                }
                else {

                    //string to concat to file location given
                    char *startFolder = "/proc/";

                    //allocates memory needed plus space for null
                    int locationLen = strlen(parsedCommand[1]) + strlen(startFolder) + 1;
                    char *getFileLoc = (char*)malloc(locationLen);

                    //gets file location
                    strcpy(getFileLoc, "/proc/");
                    strcat(getFileLoc,parsedCommand[1]);

                    FILE *readProc = fopen(getFileLoc,"r");

                    //makes sure file exists
                    if (readProc == NULL){

                        printf("Cannot find file, try command again\n");

                    }
                    else {

                        char readChar = fgetc(readProc);

                        //reads through until the end of file
                        while(readChar != EOF){
                    
                        //char readLine = getline(readProc,readLine);
                        printf("%c", readChar);
                        readChar = fgetc(readProc);

                        }
                        printf("\n");

                        fclose(readProc);
                    }

                    //frees memory 
                    free(getFileLoc);
                }
            }
            else {
            
                //passes command to be used in exe
                execute_command(parsedCommand, getCommand);

            }
        
            //frees memory
            free(getCommand);
            free(parsedCommand);

        }
        //nothing was entered by user
        else {

            //frees memory
            free(getCommand);

        }

    }
    
}

/*
    get_user_command():
    Take input of arbitrary size from the user and return to the
    user_prompt_loop()
*/
char *get_user_command() {

    int currentMax = 20;
    char *command = (char*)malloc(currentMax);  

    //get first input
    char input = fgetc(stdin);

    int index = 0;
    //while user does not input enter
    while (input != '\n') {                                  

        command[index] = input;                              

        if ((index + 1) == currentMax) { 

            //adds more memory if cstring is full
            currentMax += 20;
            command = (char*)realloc(command, currentMax);

        }

        //keeps getting input
        index++;
        input = fgetc(stdin);

    }

    //sets last index to null, string will never be empty at this point
    command[index] = '\0';

    return command;
}

/*
    parse_command():
    Take command input read from the user and parse appropriately.
    Example:
        user input: "ls -la"
        parsed output: ["ls", "-la", NULL]
    Example:
        user input: "echo     hello                     world  "
        parsed output: ["echo", "hello", "world", NULL]

    This parse command uses strtok which does not take into account 
    quotes and escape sequences. I could not get the command parsed 
    using first_uquoted_space() and unescaped to parse the command 
    in the correct format. The parse command below parses the command
    as expected but without use of escape sequences
*/
char **parse_command(char *command) {

    int currentMax = 10;
    char **parseArray = (char**)calloc(currentMax, sizeof(char*));

    //gets first string to next " "
    char *subChar = strtok(command, " ");
  
    int index = 0;
    //while there is still input to split
    while (subChar != NULL) {
        
        //array is full
        if ((index + 1) == currentMax) {
            
            //realloc array size
            currentMax += 10;
            parseArray = (char**)realloc(parseArray, currentMax * sizeof(char*));
            
        }

        //sets last index to subChar
        parseArray[index] = subChar; 

        //gets first string to next " "
        subChar = strtok(NULL, " ");

        index++;
    }
    
    //sets last index to null(subChar), array will never be full at this point
    parseArray[index] = subChar;

    return parseArray;
}

/*
    execute_command():
    Execute the parsed command if the commands are neither proc nor exit;
    fork a process and execute the parsed command inside the child process
*/
void execute_command(char **parsedCommand, char* getCommand) {

    pid_t pidExe = fork();

    //in child process
    if (pidExe == 0) {
        
        int checkExeFailed = execvp(parsedCommand[0],parsedCommand);
        
        //execvp failed returns -1
        if (checkExeFailed == -1){
            printf("Command not valid\n");
        }

        //frees memory in child
        free(getCommand);
        free(parsedCommand);

        //child exits
        exit(0);

    }
    //in parent process
    else if (pidExe > 0) {

        //waits for child
        wait(NULL);

    }
    //fork failed
    else {

        printf("Failed creating new process\n");

    }
    
}
