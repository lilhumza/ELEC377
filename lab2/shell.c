#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/stat.h>

//+
// File:	shell.c
//
// Pupose:	This program implements a simple shell program. It does not start
//		processes at this point in time. However, it will change directory
//		and list the contents of the current directory.
//
//		The commands are:
//		   cd name -> change to directory name, print an error if the directory doesn't exist.
//		              If there is no parameter, then change to the home directory.
//		   ls -> list the entries in the current directory.
//			      If no arguments, then ignores entries starting with .
//			      If -a then all entries
//		   pwd -> print the current directory.
//		   exit -> exit the shell (default exit value 0)
//				any argument must be numeric and is the exit value
//
//		if the command is not recognized an error is printed.
//-

//constants and function declaration
#define CMD_BUFFSIZE 1024
#define MAXARGS 10

int splitCommandLine(char * commandBuffer, char* args[], int maxargs);
int doInternalCommand(char * args[], int nargs);
int doProgram(char * args[], int nargs);

//+
// Function:	main
//
// Purpose:	The main function. Contains the read
//		eval print loop for the shell.
//
// Parameters:	(none)
//
// Returns:	integer (exit status of shell)
//-

int main() {

    char commandBuffer[CMD_BUFFSIZE];
    // note the plus one, allows for an extra null
    char *args[MAXARGS+1];

    // print prompt.. fflush is needed because
    // stdout is line buffered, and won't
    // write to terminal until newline
    printf("%%> ");
    fflush(stdout);

    while(fgets(commandBuffer, CMD_BUFFSIZE, stdin) != NULL) {

	// remove newline at end of buffer
	int cmdLen = strlen(commandBuffer);
	if (commandBuffer[cmdLen-1] == '\n'){
	    commandBuffer[cmdLen-1] = '\0';
	    cmdLen--;
	}

	// Split command line into words via splitCommandLine() function (Step 2)
    int nargs = splitCommandLine(commandBuffer, args, MAXARGS);
    
	// Add a null to end of array, ending index = nargs (Step 2)
    args[nargs] = NULL;

	// debugging
	//printf("%d\n", nargs);
	//int i;
	//for (i = 0; i < nargs; i++){
	//   printf("%d: %s\n",i,args[i]);
	//}
	// element just past nargs
	//printf("%d: %x\n",i, args[i]);

    // TODO: (Step 3 & 4)
    if(nargs >= 1){ // Check if 1 or more args
        int outputdIC = doInternalCommand(args, nargs); // Call doInternalCommand  (Step 3)
        if (outputdIC == 0){ //If doInternalCommand returns 0, call doProgram (Step 4)
            int outputdP = doProgram(args, nargs);
            if(outputdP == 0){ //If doProgram returns 0, print error message (Step 3 & 4)
                printf("Error: command '%s' not found \n", args[0]);
            }
        }
    }

	// print prompt
	printf("%%> ");
	fflush(stdout);
    }
    return 0;
}

////////////////////////////// String Handling (Step 1) ///////////////////////////////////

//+
// Function:	skipChar
//
// Purpose:	This function skips over a given char in a string
//		For security, will not skip null chars.
//
// Parameters:
//    charPtr	Pointer to string
//    skip	character to skip
//
// Returns:	Pointer to first character after skipped chars
//		ID function if the string doesn't start with skip,
//		or skip is the null character
//-

char *skipChar(char * charPtr, char skip){
    if (skip == '\0') { //Null terminator
        return charPtr;
    }

    while (skip == *charPtr) {
        charPtr++; //Move to next char in array
        // check for null character
        if (skip == '\0') {
            return charPtr;
        }
    }
    return charPtr;
}


//+
// Funtion:	splitCommandLine
//
// Purpose:	TODO: give descritption of function
//
// Parameters:
//	TODO: parametrs and purpose
//
// Returns:	Number of arguments (< maxargs).
//
//-

int splitCommandLine(char *commandBuffer, char *args[], int maxargs) {
   // TODO: Contents of function

  int numArgs = 0; // Counter used as an index for iterations
 
  // Skip until the first word is found (in case it starts with 0's)
  char *currentPosition = skipChar(commandBuffer, ' ');

  while (*currentPosition != '\0') {
    // Skip until next word is found
      currentPosition = skipChar(currentPosition, ' ');
    
    // Check if max args are reached, and print error if it is. If not then add the pointer position for the new word to the args array and increment numargs
      if (numArgs != maxargs) {
          args[numArgs] = currentPosition;
          numArgs++; //iterate counter

          currentPosition = strchr(currentPosition, ' '); // Store pointer to next empty space
      } else {
          fprintf(stderr, "Error maximum number of arguments reached");
          return numArgs;
      }

      if (currentPosition == NULL) {
          return numArgs;
      }

      *currentPosition = '\0'; // Set pointer to NULL
      currentPosition++; // Iterate pointer by a byte

  }
   return numArgs;
}

////////////////////////////// External Program  (Note this is step 4, complete doeInternalCommand first!!) ///////////////////////////////////

// list of directorys to check for command.
// terminated by null value
char * path[] = {
    ".",
    "/usr/bin",
    NULL
};

//+
// Funtion:	doProgram
//
/* Purpose:	To  locate an executeable for a valid user inputted cmd and execute accordingly*/
// Parameters:
//	TODO:
//  *args[] contains the array of commands and the arguments
//  nargs is the number of arguments associated with the inputted command
// Returns	int
//		1 = found and executed the file
//		0 = could not find and execute the file
//-

int doProgram(char *args[], int nargs) {
    // find the executable
    // TODO: add body.
    // Note this is step 4, complete doInternalCommand first!!!

    struct stat status; // Create an instance status of struct stat 
    char *cmd_path; // Char pointer used to allocate memory and format strings
    int currentDirIndex = 0; // Counter used as an index for directory of elements in array

    while (path[currentDirIndex] != NULL) {
        cmd_path = (char *)malloc(strlen(path[currentDirIndex]) + strlen(args[0]) + strlen("/") + 1); // Allocate enough memory for all cmd types

        // Use sprintf to fill in and format the string
        sprintf(cmd_path, "/%s", path[currentDirIndex]);
        sprintf(cmd_path + strlen(cmd_path), "/%s", "/");
        sprintf(cmd_path + strlen(cmd_path), "/%s", args[0]);

        if (stat(cmd_path, &status) == 0){
            if (S_ISREG(status.st_mode)) {
                if (status.st_mode & S_IXUSR){ //Case where file executable
                    break;
                } else {
                    fprintf(stderr, "Error the file not executable\n");
                return 0;
                }
            } else { //Phase this sequence of code out
                fprintf(stderr, "File is Executable\n");
            }
        } else {
            fprintf(stderr, "Error the file does not exist\n");
        }
        currentDirIndex++; //Iterate to next
        free(cmd_path); // Deallocate cmd_path
    }
    
    //Error handling for dir not exist
    if (cmd_path == NULL) {
        printf("Directory Does Not Exist");
        return 0;
    }

    int forkProcess = fork(); // Split process into parent and child processes

    if (forkProcess == -1) { // Error with fork() case
        fprintf(stderr, "Child process could not be created\n");
        free(cmd_path);
        return 0;
    } else if (forkProcess == 0) { // Process is the child and pid is the child
        // Execute command
        execv(cmd_path, args); // Replace running program with program at cmd_path
    } else {
        wait(NULL); // Wait for child process to finish
    }

    free(cmd_path); // Deallocate cmd_path
    return 1;
}

////////////////////////////// Internal Command Handling (Step 3) ///////////////////////////////////

// define command handling function pointer type
typedef void(*commandFunc)(char *args[], int nargs);

// associate a command name with a command handling function
struct cmdStruct {
   char *cmdName;
   commandFunc cmdFunc;
};

// prototypes for command handling functions
// TODO: add prototype for each comamand function

void exitFunc(char *args[], int nargs);
void lsFunc(char *args[], int nargs);
void cdFunc(char *args[], int nargs);
void pwdFunc(char *args[], int nargs);

// list commands and functions
// must be terminated by {NULL, NULL} 
// in a real shell, this would be a hashtable.

struct cmdStruct commands[] = {
    // TODO: add entry for each command
    {"exit", exitFunc},
    {"pwd", pwdFunc},
    {"cd", cdFunc},
    {"ls", lsFunc},
    { NULL, NULL}   // terminator
};

//+
// Function:	doInternalCommand
//
/* Purpose:	From user inputted commands, compares to the cases the shell can 
handle to determine validity, thereafter calling the related command handling functions */ 
// Parameters:
// nargs is the number of arguments in the inputted command by the user
// *args is the list of commands inputted by the user
// Returns	int
//		1 = args[0] is an internal command
//		0 = args[0] is not an internal command
//-

int doInternalCommand(char *args[], int nargs) {
    for (int i = 0; commands[i].cmdName != NULL; i++) { //Break loop on NULL cmdName
        if (strcmp(commands[i].cmdName, args[0]) == 0) { //Filter for valid commands
            commands[i].cmdFunc(args, nargs); //Execute related functions to cmd
            return 1;
        }
    }
    return 0;
}

///////////////////////////////
// comand Handling Functions //
///////////////////////////////

// TODO: a function for each command handling function
// goes here. Also make sure a comment block prefaces
// each of the command handling functions.

//CMD Handling Function for exit
void exitFunc(char *args[], int nargs) {
    exit(0); //End all via exit
}

//CMD Handling Function for pwd - print working directory
void pwdFunc(char *args[], int nargs) {
    char *cwd = getcwd(NULL, 0); // Get current working directory and store to var
    printf("Current Working Directory - cwd: %s\n", cwd);
    free(cwd); //Deallocate memory for cwd

    /*FYI Could Also Make This More Modular But Avoid Handling via 
    printf("Current Working Directory - cwd: %s\n", getcwd(NULL, 0));
    */ 

}

//CMD Handling Function for cd - change directory
void cdFunc(char *args[], int nargs) {
    char *new_directory;

    if (nargs == 1) { // If sole cd arg present
        struct passwd *pw = getpwuid(getuid()); // Retrieve password file entry of user
        if (pw->pw_dir == NULL) {
            fprintf(stderr, "Error acessing home directory\n");
        }
        new_directory = pw->pw_dir; // Directory is the home directory
    } else {
        new_directory = args[1]; // Set new directory other args passed in cd cmd
    }

    if (chdir(new_directory) != 0) {
        fprintf(stderr, "Error changing to intended directory - Check directory name for correctness\n");
    }
}

// Helper function to filter and find . thereafter being added to list or not via return flag
int findDot(const struct dirent *d) {
    if (d->d_name[0] == '.') {
        return 0;
    }
    return 1;
}

//CMD Handling Function for ls - list directory
void lsFunc(char *args[], int nargs) {
    
    struct dirent **namelist; // Array of pointers to struct dirent
    int numEnts; // Number of elements in array

    // ls -a handling
    if (nargs == 2 && strcmp(args[1], "-a") == 0) { //string comparison to determin -a flag
        numEnts = scandir(".", &namelist, NULL, NULL);     
    } else if (nargs == 1) { // ls
        numEnts = scandir(".", &namelist, findDot, NULL);
    } else {
        fprintf(stderr, "Error with number of arguments passed in cmd\n");
    }

    for (int i = 0; i < numEnts; i++) {
            //Print out each name of elements in namelist
            printf("%s\n", namelist[i]->d_name);
        }
    
}
