/**********************************************************************
 * Geoffrey Pard
 * CS 344 - 400
 * Program 3 -- SMALL SHELL
 * DUE: February 29, 2016
 *
 * This program creates a basic shell. It supports
 * three built in commands: exit, cd, status,
 * and also comments.
 *
 * Usage:
 * command [arg1 arg2 ...] [< input_file] [>output_file] [&]
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

// Program Stipulations - To be handled at command MAX_ARGS
#define MAX_CHARS 2048
#define MAX_CMD 512

// Struct for symbol flags
typedef struct {
   int bg_process;  // background '&'
   int input_direct; // input redirection '<'
   int output_direct; // output redirection '>'
}Flags;
Flags flag;

// Function Prototypes
void catchint(int signo);
void cmdLine_input();
void check_running();

// Variables
int bg_pids[64]; // array background processes
int num_bgProcess; // count background processes
char MAX_ARGS[512];
int exit_code = 0;

/******************************************************************
 *                         main
 ******************************************************************/
int main(int argc, char *argv[])
{
   int fd;  // used for file manipulation

   /*-------------------------
      SIGACTION STRUCTURES
   ------------------------*/
   // FG -- Foreground Processes
   struct sigaction fg_act;
   fg_act.sa_handler = catchint;
   fg_act.sa_flags = 0;
   sigfillset(&(fg_act.sa_mask));
   sigaction(SIGINT, &fg_act, NULL);
   
   // BG -- Background Processes
   struct sigaction bg_act;
   bg_act.sa_handler = catchint;
   bg_act.sa_flags = 0;
   sigfillset(&(bg_act.sa_mask));
   sigaction(SIGINT, &bg_act, NULL);

   // Rest of Time
   struct sigaction restOfTheTime_act, initial_act;
   initial_act.sa_handler = catchint;
   initial_act.sa_flags = 0;
   sigfillset(&(initial_act.sa_mask));
   sigaction(SIGINT, &initial_act, &restOfTheTime_act);

   int status = 0;
   int run_prog = 1; // run shell program while true
   while (run_prog)
   {
      
      // Establish for each iteration
      sigaction(SIGINT, &initial_act, &restOfTheTime_act);
      sigaction(SIGINT, &bg_act, NULL);
      sigaction(SIGINT, &fg_act, NULL);

      check_running(); // checks whats going on in the background
      cmdLine_input(); // manipulates user input



      //---This Section: Handles 'Built-In Functions exit, cd, status----

      // Precondition: Ignore Comments
      if (strncmp(MAX_ARGS, "#", 1) == 0) continue; 

      // Other Built-Ins
      if (strcmp(MAX_ARGS, "exit") == 0)
      {
         run_prog = 0; // terminate loop
      }
      else if (strcmp(MAX_ARGS, "status") == 0)
      {
         // get and print exit status of user actions
         status = WEXITSTATUS(status);
         printf("exit value %d\n", status);
         
      }
      else if (strcmp(MAX_ARGS, "fake_test") == 0)
      {
         status = 1;
      }
      else if (strncmp("cd", MAX_ARGS, 2) == 0)
      {
         // Get Current Working Directory
         char cwd[1024];
         getcwd(cwd, sizeof(cwd));
       
         // Get Directory to Change into from User input
         char src[32];
         strcpy(src, MAX_ARGS);
         int num = strlen(src);
         
         if (num > 2)
         {
            num = num - 3; // Index beyond "cd "
            char temp[32]; // temp for indexing
            int i = 0;

            for (i = 0; i < num; i++)  // slide appropriate characters
            {
               temp[i] = src[i+3];
            }
  
            strcpy(src, temp); // move temp into source
            strcat(cwd, "/"); 
            strcat(cwd, src); // Build path
         
            // Use updated cwd to change directory
            chdir(cwd);
         }
         else
         {
            // Typing just cd, switches to HOME
            char *home;
            home = getenv("HOME");
            chdir(home);
         }
      }
      // -- NEXT SECTION:  All other Commands --
      else
      {
         pid_t spawnpid; // to split processes
        
         
         // Insert Commands into Array
         char * command;
         const char s[] = " ";

         char *args[MAX_CMD];
 
         command = strtok(MAX_ARGS, s); // use a space as delimiter
         args[0] = command; // initiate
         int userArgs = 1; // start count
         args[userArgs] = strtok(NULL, s);
   
         // Loop through remaining commands, if any
         while (args[userArgs] != NULL)
         {
            userArgs = userArgs + 1;
            args[userArgs]  = strtok(NULL, s);
         }

         /*********Testing***********************
         int j;
         size_t size = *(&args + 1) - args; 
         printf("Size: %d\n", size);
         for (j = 0; j < userArgs; j++)
         {
            printf("%s\n", args[j]);
         }
         */


         if (flag.bg_process == 1) // check background flag
         {
            spawnpid = fork(); // spawn background process
            if (spawnpid < 0)
            {
               perror("Fork failed.");
               exit(1);
            }
         
            if (spawnpid == 0) // Child
            {
               int i;
               for (i = 0; i < userArgs; i++)
               {
                  if (flag.input_direct == 1) // redirect flag
                  {
                     // If file is available read it; otherwise report error
                     fd = open(args[userArgs+2], O_RDONLY, 0);
                     if (fd == -1)
                     {
                        printf("smallsh: cannot open %s for input\n", args[userArgs+1]);
                        exit(1);
                     }
                     else
                     {
                        // Everything's working; execute
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        execvp(command, &command);
                     }
                  }
   
                  if (flag.output_direct == 1) // redirect flag
                  {
                     // Try to create file for command; set permission
                     fd = creat(args[userArgs+2], 0644);
                     if (fd == -1)
                     {
                        printf("smallsh: cannot open %s for input\n", args[userArgs+1]);
                        exit(1);
                     }
                     else
                     {
                        // Everything's ok; execture
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        execvp(command, &command);
                     }
                  }
               }
               // Command Only - No redirection
               if (flag.input_direct == 0 && flag.output_direct == 0)
               {
                  execvp(command, args);
               }
               // Can't find command
               printf("%s: no such file or directory\n", command);
            }
            else
            {
               // Keep track of background processes; insert into array
               int status_1;
               int process;
               printf("background pid is %d\n", spawnpid);
               bg_pids[num_bgProcess] = spawnpid;
               num_bgProcess = num_bgProcess + 1;
               process = waitpid(spawnpid, &status_1, WNOHANG);
               continue;
            }
         }
         else // foreground processes
         {   
            spawnpid = fork();
            if (spawnpid < 0)
            {
               perror("Fork failed.");
               exit(1);
            }
            
            if (spawnpid == 0) // Child
            { 
               int i; 
               for (i = 0; i < userArgs; i++)
               {
                  if (flag.input_direct == 1) // redirect flag
                  {
                     // If available read file; otherwise print error
                     fd = open(args[i+2], O_RDONLY, 0);
                     if (fd == -1)
                     {
                        printf("smallsh: cannot open %s for input\n", args[i+2]);
                        exit(1);
                     }
                     else
                     {
                        // It worked; execute action
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        execvp(command, &command);
                     }
                  }
   
                  if (flag.output_direct == 1) // redirect flag
                  {
                     // Try to create file; otherwise print error
                     fd = creat(args[i+2], 0644);
                     if (fd == -1)
                     {
                        printf("smallsh: cannot open %s for input\n", args[i+2]);
                        exit(1);
                     }
                     else
                     {
                        // Good to go; execute command
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        execvp(command, &command);
                     }
                  }
               }
 
               // Just a command - no redirection
               if (flag.input_direct == 0 && flag.output_direct == 0)
               {
                  execvp(command, args);
               }
              
               // Don't understand command
               printf("%s: no such file or directory\n", command);
            }
            else
            {
               // Keep track of ending processes - set exit code
               int status_1;
               waitpid(spawnpid, &status, 0);

               if (WIFEXITED(status_1))
               {
                  exit_code = WEXITSTATUS(status_1);
               }
            }      
         }
      }  
      // Reset flags   
      flag.bg_process = 0;
      flag.input_direct = 0;
      flag.output_direct = 0;

      signal(SIGINT, SIG_IGN); 
   }
   return 0;

}

/*************************************************************************
 *                         catchint
 *
 * This function is used to capture signals from the user
 ************************************************************************/
void catchint(int signo)
{
   //Print signal number
   printf("terminated by signal %d\n", signo);

}
/*************************************************************************
 *                        cmdLine_input
 *
 * Analyze user input, flush input/output, judge if a process should be
 * run in the background.
 ************************************************************************/
void cmdLine_input()
{
   // Flush StdIn, StdOut
   fflush(stdout);
   fflush(stdin);
   printf(": ");
   fflush(stdin);


   if (fgets(MAX_ARGS, sizeof(MAX_ARGS), stdin) != NULL)
   {
      char *ret;
      ret = strchr(MAX_ARGS, '\n'); // Library function to search for char
      *ret = '\0'; // dereference and change to null terminating char

      flag.bg_process = 0;

      // ----CHECK FOR SPECIAL CHARACTERS-----

      if ((ret = strchr(MAX_ARGS, '&')) != NULL) // if not null, tally bg
      {
         *ret = '\0';  // Remove '&' and terminate string
         flag.bg_process = 1; 
      }
      
      
      if ((ret = strchr(MAX_ARGS, '<')) != NULL) // if not null, set flag
      {
         flag.input_direct = 1; 
     
      }
    
      if ((ret = strchr(MAX_ARGS, '>')) != NULL) // if not null, set flag
      {
         flag.output_direct = 1;
        
      }

   }
}

/*************************************************************************
 *                        check_running
 *
 * Used to check up on the status of processes running in the background
 ************************************************************************/
void check_running()
{
   int status;
  
   int i;
   // This loop cycles through the array background processes
   // and sets exit or term statuses
   for (i = 0; i < num_bgProcess; i++)
   {
      if (waitpid(bg_pids[i], &status, WNOHANG) > 0)
      {
         if (WIFEXITED(status))
         {  
            printf("background pid %d is done: exit value %d\n", bg_pids[i],
             WEXITSTATUS(status));
         }
         if (WIFSIGNALED(status))
         {
            printf("background pid %d is done: terminated by signal %d\n", bg_pids[i],
             WTERMSIG(status));
         }
      }
   }  
}

