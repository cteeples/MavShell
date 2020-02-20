/*
    Name: Christian Teeples
    ID: 1001122564
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line
#define NUM_OF_STRINGS 15

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments

//I am making my history array global variables so that I don't die in frustration
char history_commands[NUM_OF_STRINGS][MAX_COMMAND_SIZE];

int historyCommandCount = 0;

int history_pids[NUM_OF_STRINGS];

int historyPidCount = 0;

pid_t pid;

void runCommand(char *token[], char * cmd_str);

int doesEndInSemicolon(const char *stringVal);

void printHistory(char name[][MAX_COMMAND_SIZE], int counter);

void printPidHistory(int name[], int counter);

static void handle_signal (int sig );

int main(int argc, char *argv[])
{
  //int storedTokenIndex = 0;

  //char storedToken[50][15];

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  struct sigaction act;
 
  /*
    Zero out the sigaction struct
  */ 
  memset (&act, '\0', sizeof(act));
 
  /*
    Set the handler to use the function handle_signal()
  */ 
  act.sa_handler = &handle_signal;
 
  /* 
    Install the handler for SIGINT and SIGTSTP and check the 
    return value.
  */ 
  if (sigaction(SIGINT , &act, NULL) < 0) 
  {
    perror ("sigaction: ");
    return 1;
  }

  if (sigaction(SIGTSTP , &act, NULL) < 0) 
  {
    perror ("sigaction: ");
    return 1;
  }

  while( 1 )
  {
    //signal(SIGTSTP, sighandler);

    // Print out the msh prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );              

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    //char *working_root = working_str;

    //token = tokenize(working_root);  

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    //We don't want to try to run the command if the user just pressed 'enter'
    if (token[0] != NULL)
    {
      runCommand(token, cmd_str);
    }

    //we are going to save the current command into the history_commands array and up the counter for the next save
    if (historyCommandCount < 15)
    {
      strcpy(history_commands[historyCommandCount], cmd_str);
      historyCommandCount++;
    }
    else //if we have already filled up all 15 spots in our history array, we need to take out the oldest history and move everything back
    {
      int i = 0;
      while (i != 14) //iterates through all strings in the history_command array and will move the elements back one
      {
        strcpy(history_commands[i], history_commands[i + 1]);
        ++i;
      }
      strcpy(history_commands[14], cmd_str);
    }
  
    free( working_str );

  }
  return 0;
}

void runCommand(char *token[], char * cmd_str)
{
    //char *temp[MAX_NUM_ARGUMENTS]; //I have a temp array for recursive calls of runCommand for !n 
                                   //and commands that end in semicolon
    ///////////////////////////////////////////R E Q   5//////////////////////////////////////////////////////////////////////////////
    if (strcmp(token[0], "exit") == 0 || strcmp(token[0], "quit") == 0) // If the user types "exit" or "quit" we will exit the program
    {
      exit(0); //exit with status of 0
    }
    ///////////////////////////////////////////R E Q   1 3////////////////////////////////////////////////////////////////////////////
    else if (strcmp(token[0], "cd") == 0) // If the user types in cd, we want to change directories to the second argument
    {
      chdir(token[1]);
    }
    ///////////////////////////////////////////R E Q   1 5////////////////////////////////////////////////////////////////////////////
    else if (token[0][0] == '!')
    {
      //char * new_cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
      char *newToken[MAX_NUM_ARGUMENTS];
      char stringNum1 = token[0][1]; //getting the number after '!' so that we can index the history array
      char stringNum2 = token[0][2];
      int historyIndex1 = stringNum1 - '0'; //converts char to int
      int historyIndex2 = (stringNum2 == '\0') ? 0 : stringNum2 - '0'; //we are checking for whether the integer is 1 or 2 digits
      int historyIndex = (historyIndex2 == 0) ? historyIndex1 : ((historyIndex1 * 10) + historyIndex2);
      //I couldn't figure out how to pass the tokenizing process as a function, so here is some unfortunate repeated code
      //We are retokenizing the specific string command from the history array
      int   token_count = 0;                                 
      char *arg_ptr;                                                                                             
      char *working_str  = history_commands[historyIndex];              
      //char *working_root = working_str;
      while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
                (token_count<MAX_NUM_ARGUMENTS))
      {
        newToken[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
        if( strlen( newToken[token_count] ) == 0 )
        {
          newToken[token_count] = NULL;
        }
          token_count++;
      }
      runCommand(newToken, cmd_str); //recursive call of running token
    }
    ///////////////////////////////////////////R E Q   1 5////////////////////////////////////////////////////////////////////////////
    else if (strcmp(*token, "history") == 0)
    {
      printHistory(history_commands, historyCommandCount);
    }
    ///////////////////////////////////////////R E Q   1 4////////////////////////////////////////////////////////////////////////////
    else if (strcmp(*token, "listpids") == 0)
    {
      printPidHistory(history_pids, historyPidCount);
    }
    ///////////////////////////////////////////R E Q   8//////////////////////////////////////////////////////////////////////////////
    else if (strcmp(*token, "bg") == 0)
    {
      kill(pid, SIGCONT);
    }
    ///////////////////////////////////////////R E Q   1 6////////////////////////////////////////////////////////////////////////////
    //checking that every token ends in a semicolong or is null
    //teacher specified we only have to be able to run 5 commands in this way
    else if ((doesEndInSemicolon(token[0])) && (doesEndInSemicolon(token[1]) || token[1] == NULL) &&
    (doesEndInSemicolon(token[2]) || token[2] == NULL) && (doesEndInSemicolon(token[3]) || token[3] == NULL) && 
    (doesEndInSemicolon(token[4]) || token[4] == NULL))
    {
      //char * new_cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
      char *newToken[MAX_NUM_ARGUMENTS];
      
      //I couldn't figure out how to pass the tokenizing process as a function, so here is some unfortunate repeated code
      //We are retokenizing the specific string command from the history array
      int   token_count = 0;                                 
      char *arg_ptr;      
      //T O D O: figure out how to take off last char of a string in c... we will then take this modified string and tokenize it than 
      //loop back until there are no more commands
      int i = 0;
      while(newToken != NULL)
      {
        char *temp = strtok(cmd_str, ";"); //spliting up the whole commands into each individual command
        char *working_str  = temp;       
        //char *working_root = working_str;
        while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
                  (token_count<MAX_NUM_ARGUMENTS))
        {
          newToken[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
          if( strlen( newToken[token_count] ) == 0 )
          {
            newToken[token_count] = NULL;
          }
            token_count++;
            ++i;
        }
        runCommand(newToken, cmd_str); //recursive call of running token
      }
      
    }
    ///////////////////////////////////////////R E Q   1 0////////////////////////////////////////////////////////////////////////////
    else
    {
        pid = fork(); //save pid

        if (historyPidCount < 15)
        {
          history_pids[historyPidCount] = pid;
          historyPidCount++;
        }

        else
        {
          int i = 0;
          while (i != 14) //iterates through all pids into history_pids array and will move the elements back one
          {
            history_pids[i] = history_pids[i + 1];
            ++i;
          }
          history_pids[14] = pid;
        }

        if (pid == 0)
        {
          int ret = execvp(token[0], token);
            if (ret == -1)
                printf("%s: Command not found \n", *token); //printing invalid options
        }
        else
        {
          int status;
          wait(&status);
        }
    }
}

int doesEndInSemicolon(const char *stringVal)
{
  if(stringVal[strlen(stringVal + 1)] == ';' && *stringVal)
    return 1;
  else
    return 0;
}

void printHistory(char name[][MAX_COMMAND_SIZE], int counter)
{
    int totalPrints = (counter <= 15) ? counter : 15;
    int i;
    for(i = 0; i < totalPrints; i++)
    {
        printf("%d: %s", i, name[i]); //I don't include \n because that is already in the cmd_str
    }
}

void printPidHistory(int name[15], int counter)
{
  int totalPrints = (counter <= 15) ? counter : 15;
  int i;
  for(i = 0; i < totalPrints; i++)
  {
    printf("%d: %d \n", i, name[i]);
  }
}

static void handle_signal (int sig )
{
  /*
   Determine which of the two signals were caught and 
   print an appropriate message.
  */
  switch( sig )
  {
    case SIGINT:
    break;

    case SIGTSTP: 
    break;
  }
}
