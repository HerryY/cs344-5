/********************************************************************
 * Geoffrey Pard
 * CS 344 - 400
 * Program 2
 * Due 2/7/16
 *
 * Program 2: Adventure -- Find the end room
 * in the fewest number of steps.
 * ******************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>

#define ROOM_NAMES 10
#define MIN_CONNECT 3
#define MAX_CONNECT 6
#define USED_ROOMS 7


/*************************************************************
 * Room Structure
 ************************************************************/
struct RM_Types
{
   char * start;
   char * middle;
   char * end;
};

/*************************************************************
 * Function Prototypes:
 * **********************************************************/
void intro();
char * mkRoomDir(int);
struct RM_Types make_rooms(char *);
void adventure(struct RM_Types);
void randomize(char **, size_t);

/*************************************************************
 *                         MAIN
 *************************************************************/
int main(void)
{
   // Seed Random Number Generator 
   srand(time(NULL));

   // Introduction
   intro();

  // Find the process ID to be used with directory creation
  int pid = getpid();
 
  // Create the directory and fill with room files
  char * room_dir = mkRoomDir(pid);
  struct RM_Types user_rooms = make_rooms(room_dir);

  // Loop until game is completed
  adventure(user_rooms);

  free(room_dir);

   return 0;
}

/*******************************************************************
 *                           intro
 *
 * Short Description to Start the Game
 *******************************************************************/
void intro()
{
   printf("WELCOME TO ADVENTURE!\n\n");
   printf("Find the end room in the fewest steps possible\n\n");
}

/******************************************************************
 *                           mkRoomDir
 *
 * This function accepts the current process id and allocates space
 * for a new directory to hold game play files.
 *******************************************************************/
char * mkRoomDir(int pid)
{

   // Set aside new directory; verify allocation
   int buffer = 20;  
   char * new_dir = malloc(buffer);
   assert(new_dir != 0);

   char * username = "pardg.rooms.";  // name directory

   // Stringify named pieces 
   snprintf(new_dir, buffer, "%s%d", username, pid);

   // mkdir in 'C' from StackOverflow: http://goo.gl/qdsK0u 
   struct stat dir = {0};

   if (stat(new_dir, &dir) == -1)
   {
      mkdir(new_dir, 0755); // make directory and set permissions 
   }

   return new_dir;  

}

/*********************************************************************
 *                      make_rooms
 *
 * Name and Create individual rooms to program specifications
 *********************************************************************/
struct RM_Types make_rooms(char * new_room_dir)
{

   // Room List
   char * rmNames[ROOM_NAMES];
   rmNames[0] = "robbins";
   rmNames[1] = "palahniuk";
   rmNames[2] = "chabon";
   rmNames[3] = "hempel";
   rmNames[4] = "vonnegut";
   rmNames[5] = "chandler";
   rmNames[6] = "franzen";
   rmNames[7] = "auster";
   rmNames[8] = "murakami";
   rmNames[9] = "amis";

   // structure for created rooms
   struct RM_Types rms;
   rms.middle = new_room_dir;

   // Initialize buffer and Verify allocation
   int buffer = 128;
   char * file = malloc(buffer);
   assert(file != 0);
  
   
   // For game randomness aspect
   randomize(rmNames, ROOM_NAMES);
   
   // Get 7 Rooms; make files
   int i;
   for (i = 0; i < USED_ROOMS; i++)
   {
      snprintf(file, buffer, "%s/%s", new_room_dir, rmNames[i]);
      FILE * input = fopen(file, "w"); // write to file
      if (input == NULL)
      {
         perror("Could Not Open File.\n");
         exit(1);
      }
      else  // All good, write the names
      {
         fprintf(input, "ROOM NAME: %s\n", rmNames[i]); 
      }
      fclose(input);
   }
   
   // Fill an array with selected rooms
   char * saved[USED_ROOMS];

   for (i = 0; i < USED_ROOMS; i++)
   {
      saved[i] = rmNames[i];
   } 

   // Randomize Start Room and End Room
   int begin_room = rand() % USED_ROOMS;
   int end_room = rand() % USED_ROOMS;

   // If random is the same, try again
   while (begin_room == end_room)
   {
      end_room = rand() % USED_ROOMS;
   } 


   char * connect; // track room connections
   int counter; // count connections
   int j, k; // loops

   for (i = 0; i < USED_ROOMS; i++)
   {
      randomize(saved, USED_ROOMS); // Change up possible connections
      snprintf(file, buffer, "%s/%s", new_room_dir, rmNames[i]);

      FILE * input = fopen(file, "a"); // append to room file
      if (input == NULL)
      {
         perror("Could Not Open File.\n");
         exit(1);
      }
      else
      {
         // Randomize connections based on program specs
         counter = rand() % 4 + 3;

         // Loop to create room connections
         k = 0;
         for (j = 0; j < counter; j++)
         {
            connect = saved[k];
            if (connect == rmNames[i]) // can't connect to itself
            {
               k++;
               connect = saved[k];
            }
            fprintf(input, "CONNECTION %d: %s\n", j+1, connect);
            k++;
         }
         // Check to see if room is the start or end room
         // If so, store as such in file
         if (i == begin_room)
         {
            fprintf(input, "ROOM TYPE: START_ROOM\n");
            rms.start = rmNames[i];
         }
         else if (i == end_room)
         {
            fprintf(input, "ROOM TYPE: END_ROOM\n");
            rms.end = rmNames[i];
         }
         else  // Everything else is a mid room
         {
            fprintf(input, "ROOM TYPE: MID_ROOM\n");
         }
      }
      fclose(input);

   }
   free(file);

   return rms;
}    

void adventure(struct RM_Types room_in)
{
   // Initiate Beginning Rooms, Files
   char * first_room = room_in.start;
   char * middle_room = room_in.middle;
   char * last_room = room_in.end;

   int counter = 0;
   int flag; // boolean
   int buffer = 128;
   int i;

   // Memory Allocation -- Room Acess, File Storage, and Path
   char (*steps)[15] = malloc(sizeof *steps * 8);
   assert(steps != 0);

   char (*contents)[15] = malloc(sizeof *contents * 8);
   assert(contents != 0);

   char user_choice[15]; // Where to?

   char * file = malloc(buffer);
   assert(file != 0);

   // Loop Until Current Room and End Room Match
   int lines;
   while (!(strcmp(first_room, last_room)) == 0)
   {
      snprintf(file, buffer, "%s/%s", middle_room, first_room); 
      FILE * input = fopen(file, "r"); // read

      // Track Room Connections
      int room_connects = 0;

      if (input)
      {
         while ((lines = getc(input)) != EOF)
         {
            if (lines == '\n')
            {
               room_connects++;
            }
         }
      }  
      room_connects-= 2; // Less two for Room Heading: name, type

      // File Navigation
      char str[20];
      fseek(input, 11, SEEK_SET);
      fgets(str, 20, input);
   
      // Remove newlines
      int length = strlen(str);
      if (str[length - 1] == '\n')
      {
         str[length - 1] = 0;
      }
      strcpy(contents[0], str);
      
      // Check for connections and store
      for (i = 1; i <= room_connects; i++)
      {
         fseek(input, 14, SEEK_CUR);
         fgets(str, 20, input);
         
         // Remove newlines
         length = strlen(str);
         if (str[length - 1] == '\n')
         {
            str[length - 1] = 0;
         }
         strcpy(contents[i], str);
      }   

      // Validate Contents, Compare, and Print to Screen
      flag = 0;
      while (flag != 1)
      {
         // Status Update
         printf("CURRENT LOCATION: %s\n", contents[0]);
         printf("POSSIBLE CONNECTIONS: ");
   
         for (i = 1; i <= room_connects; i++)
         {
            if (i == room_connects)
            {
               printf("%s.\n", contents[i]);
            }
            else
            {
               printf("%s, ", contents[i]);
            }
         }

         // Choice
         printf("WHERE TO?>");
         scanf("%s", user_choice);
  
         for (i = 1; i <= room_connects; i++)
         {
            if (strcmp(user_choice, contents[i]) == 0)
            {
               flag = 1;
               first_room = user_choice;
            }
         }
         // No Matches, Try Again
         if (flag != 1)
         {
            printf("\nHUH? I DON'T UNDERSTAND THAT ROOM.  TRY AGAIN.\n\n");
         }
      }

      printf("\n");
      strcpy(steps[counter], first_room); // store rooms visited
      counter++; 
      fclose(input);
   }

   // End Room Selected 
   printf("YOU HAVE FOUND THE END ROOM. CONGRATS!\n"); 
   printf("YOU TOOK %d STEPS.  YOUR PATH TO VICTORY WAS:\n", counter);
   for (i = 0; i < counter; i++)
   {
      printf("%s\n", steps[i]);
   }

   free(steps);
   free(contents);
   free(file);

}

/*********************************************************************
 *                         randomize
 *
 * Function used to randomize rooms used and number of connections
 *********************************************************************/
void randomize(char **list_in, size_t num)
{
   struct timeval tv;

   gettimeofday(&tv, NULL);

   int usec = tv.tv_usec;
   srand48(usec);

   if (num > 1) 
   {
      size_t i;
      for (i = num - 1; i > 0; i--)
      {
         size_t j = (unsigned int) (drand48()*(i+1));
         char * t = list_in[j];
         list_in[j] = list_in[i];
         list_in[i] = t;
      }
   }
}
