//Written by Phillp Taylor, Dan Kendall.
//Licensed under the GPL Version 2.

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>
#include <errno.h>

#include "autojump.h"

//Our save file. This is appended
//to the home direcory.
#define AUTOJUMP_FILENAME "/.autojump"

//this is how often we read / write
//our structure to the file system
#define AUTOJUMP_SYNC_TIME_SECONDS (5 * 60)

//internal size of directories.
#define AUTOJUMP_DIR_SIZE 40

#define MAX_LINE_SIZE 512

//This is our primary data structure.
//An array called jumprecs with each
//record containing a directory spec.
struct dirspec
{
	char *path;

	//total amount of time spent inside
	//the directory.
	unsigned int time;

	//timestamp of last access.
	unsigned int last_accessed;

};

struct dirspec *jumprecs[AUTOJUMP_DIR_SIZE];

//we sync to a file, after a given timeframe
//(default 5 minutes) when 'cd' or 'j' is invoked.
//The sync is a two way process so that multiple
//terminals don't trample on each other's records.
void sync_to_file();

//these functions are used in the scoring system to
//choose the best match for a record or the lowest
//match when we want to eliminate unused directories.
unsigned int get_score(int i);
int get_best_score(struct dirspec *array[], int size);

//last criteria passed into jump() so running "j" performs
//the search again, alternating between the best two matches
char *last_criteria;


char *current_directory;
//the time we entered the current directory.
unsigned int entered;

//time we last syncronised our internal
//array with the disk.
unsigned int last_sync;

void autojump_init()
{
  int i;

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
    jumprecs[i] = NULL;

  entered = 0;
  last_sync = 0;
  current_directory = NULL;

  sync_to_file();
}


int get_best_score(struct dirspec *array[], int size)
{
  int best = -1;
  int i;

  for (i = 0; i < size; i++)
  {
    if (array[i] != NULL)
    {
      if (get_score(i) > get_score(best))
        best = i;
    }
  }

  //best can return -1, meaning the entire array is empty.
  return best;
}

void autojump_directory_changed(char *new_path)
{
  int i;
  int now;
  int found = 0;
  int null_found = 0;
  int lowest_scoring_item = 0;

  char *path;

  //sync if need be.
  sync_to_file();

  now = time(NULL);

  //update the score for the directory we just left.
  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
      if (strcmp(jumprecs[i]->path, current_directory) == 0)
      {
        jumprecs[i]->time += (now - entered);
        jumprecs[i]->last_accessed = now;
      }
    }
  }

  //note that we are now in a new directory
  free(current_directory);
  current_directory = (char*) malloc ((sizeof(char) * strlen(new_path)) + 1);
  strcpy(current_directory, new_path);
  entered = now;

  //add this new directory to the array if it
  //doesn't exist.
  found = 0;
  null_found = -1;

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
      if (strcmp(jumprecs[i]->path, current_directory) == 0)
      {
         found = 1;
         break;
      }
    }
    else
    {
      if (null_found == -1)
         null_found = i;
    }
  }

  //not found, but there is space...
  if (found == 0 && null_found != -1)
  {
     jumprecs[null_found] = (struct dirspec*) malloc (sizeof(struct dirspec));
     jumprecs[null_found]->path = (char*) malloc ((sizeof(char) * strlen(new_path)) + 1);
     strcpy(jumprecs[null_found]->path, new_path);
     jumprecs[null_found]->time = 0;
     jumprecs[null_found]->last_accessed = now;
  }
  else if (found == 0 && null_found == -1)
  { //lowest score gets eliminated!
    lowest_scoring_item = 0;

    for (i = 1; i < AUTOJUMP_DIR_SIZE; i++)
    {
      //extra test to make sure we aren't removing the current directory.
      if (get_score(lowest_scoring_item) > get_score(i) && strcmp(jumprecs[i]->path, current_directory) != 0)
        lowest_scoring_item = i;
    }

    free(jumprecs[lowest_scoring_item]->path);
    jumprecs[lowest_scoring_item]->path = (char*) malloc ((sizeof(char) * strlen(new_path)) + 1);
    strcpy(jumprecs[lowest_scoring_item]->path, new_path);
    jumprecs[lowest_scoring_item]->time = 0;
    jumprecs[lowest_scoring_item]->last_accessed = now;

  }

}

unsigned int get_score(int i)
{
  if (jumprecs[i] == NULL)
    return -1;

  //scale last_accessed down to "days". we don't someone to
  //go on holiday for a week to find out that new directories
  //crush old directories in last_accessed scores.
  return ((jumprecs[i]->last_accessed % 3600 % 24 % 365) + (jumprecs[i]->time % 120));

}

char *autojump_jump(char *criteria)
{

  int i;
  int match;
  int best_match = -1;

  if (criteria == NULL && last_criteria == NULL)
    return;
  else if (criteria == NULL && last_criteria != NULL)
    criteria = last_criteria;


  if (last_criteria != NULL)
      free(last_criteria);

  last_criteria = (char*) malloc ((sizeof(char) * strlen(criteria)) + 1);
  strcpy(last_criteria, criteria);

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
       //don't jump to the current directory, use the second best instead.
       if (strcmp(jumprecs[i]->path, current_directory) != 0)
       {
         //match each record, if it is a match, mark it if it has the
         //best score.
         if (strstr(jumprecs[i]->path, criteria) != NULL)
         {
           if (best_match == -1)
              best_match = i;
           else
           {
              if (get_score(i) > get_score(best_match))
              {
                best_match = i;
              }
           }
         }
       }
    }
  }

  if (best_match == -1)
  {
    printf("no matches.\n");
    return NULL;
  }
  else
  {
    printf("%s\n", jumprecs[best_match]->path);
    return jumprecs[best_match]->path;
  }
}

void autojump_jumpstat()
{
  int i;
  unsigned int score;
  unsigned int total = 0;

  sync_to_file();

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
      score = get_score(i);
      printf("score: %d\t: %s\n", score, jumprecs[i]->path);
      total += score;
    }
  }

  printf("Total: %d\n", total);
}

void sync_to_file()
{
  struct flock fl;       //used to lock the file
  FILE *f_handle;
  int now;
  int i;

  int read_error = 0;   //used to denote parsing errors
  char filename[512];
  int found;
  char *buffer;
  char *delim_one, *delim_two;
  int total_time, last_accessed;
  int sz;
  char *path;

  //this is the array we use for merging our data
  struct dirspec *merge_array[AUTOJUMP_DIR_SIZE * 2];
  int pos = 0;

  //first step is the check we actually need
  //to sync.
  now = time(NULL);

  if ((now - last_sync) < AUTOJUMP_SYNC_TIME_SECONDS)
    return;

  //initialise merge_array
  for (i = 0; i < AUTOJUMP_DIR_SIZE * 2; i++)
	merge_array[pos] = 0;

  //attempt to get a file lock
  fl.l_type   = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 0;
  fl.l_pid    = getpid();

  buffer = getenv("HOME");
  strcpy(filename, buffer);
  strcat(filename, AUTOJUMP_FILENAME);

  //open for reading and writing
  f_handle = fopen(filename, "r+");
  if (f_handle == NULL)
  {
    //not critical. we can still write to the file
    if (fopen(filename, "w") == NULL)
    {
      printf("warning: can't read or write to %s\nNo persistence\n", filename);
      return;
    }
    else
      read_error = 1;
  }

  //we use file locks to ensure we don't mash the
  //file when different shells merge their arrays
  //together.
  if (fcntl(fileno(f_handle), F_SETLK, &fl) == -1)
  {
    //if the file is locked, set a sync for a
    //random time into the future to try and
    //minimise collisions.
    srand(time(NULL));

    last_sync += (rand() % 10);

    if (read_error == 0)
      fclose(f_handle);
      return;
  }
  
  buffer = (char*) malloc (sizeof(char) * MAX_LINE_SIZE);

  if (read_error == 0)
  {
    while (fgets(buffer, MAX_LINE_SIZE - 1, f_handle) == buffer && read_error == 0)
    {
      *(buffer + (strlen(buffer) - 1)) = '\0';
  
      //parse out the time.
      delim_one = strstr(buffer, ":");
      if (delim_one == NULL)
      {
        read_error = 1;
        break;
      }

      //parse out the last_accessed value
      delim_two = strstr(delim_one + 1, ":");
      if (delim_two == NULL)
      {
        read_error = 1;
        break;
      }
 
      errno = 0;
      total_time = strtol(buffer, &delim_one, 10);
      last_accessed = strtol(delim_one + 1, &delim_two, 10);
      printf("errno: %d\n", errno);
      if (errno != 0)
      {
        read_error = 1;
        break;
      }
  
      //rest of variable is path
      sz = strlen(delim_two);
      path = (char*) malloc (sizeof(char) * sz);
      strcpy(path, delim_two + 1);
  
      printf("path (%d): %s\n (time %d, last: %d)\n", strlen(path), path, total_time, last_accessed);
  
      merge_array[pos] = (struct dirspec*) malloc (sizeof(struct dirspec));
      merge_array[pos]->path = path;
      merge_array[pos]->time = total_time;
      merge_array[pos]->last_accessed = last_accessed;
      pos++;
  
    }
  
    free(buffer);

    if (read_error == 0)
    {
      //now merge our own data into the array.
      for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
      {
    
        if (jumprecs[i] == NULL)
          break;
    
        found = 0;
    
        for (pos = 0; (pos < (AUTOJUMP_DIR_SIZE * 2)) && found == 0; pos++)
        {
          if (merge_array[pos] == NULL)
          {
             merge_array[pos] = jumprecs[i];
             found = 1;
             printf("break...\n");
             break;
          }
          else
          {
              printf("%d: %s AGAINST %d: %s\n", i, jumprecs[i]->path, pos, merge_array[pos]->path);
    	      if (strcmp(jumprecs[i]->path, merge_array[pos]->path) == 0)
    	      {
    		 if (jumprecs[i]->time < merge_array[pos]->time)
    		    jumprecs[i]->time = merge_array[pos]->time;
    		 if (jumprecs[i]->last_accessed < merge_array[pos]->last_accessed)
    		    jumprecs[i]->last_accessed = merge_array[pos]->last_accessed;
    		 found = 1;
    	      }
          }
        }
      }
    
      //merge array contains all records we care about. take the 40 best records back!
      //todo: move current_directory across first for safety reasons?
      pos = 0;
    
      while (pos < AUTOJUMP_DIR_SIZE)
      {
        found = get_best_score(merge_array, (AUTOJUMP_DIR_SIZE * 2));
    
        if (found == -1)
          break;
        else
        {
          jumprecs[pos++] = merge_array[found];
          merge_array[found] = NULL;
        }
      }

      //free memory used during load.
      for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
      {
        if (jumprecs[i] != NULL)
        {
          if (jumprecs[i]->path != NULL)
            free(jumprecs[i]->path);
          free(jumprecs[i]);
        }
      }
    }
  }

  if (read_error == 1)
    printf("read error occured during sync_to_file\n");

  //go back to the beginning of the file
  //and write the new contents out.
  fseek(f_handle, 0, SEEK_SET);

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
      printf("%d:%d:%s\n", jumprecs[i]->time, jumprecs[i]->last_accessed, jumprecs[i]->path);
      fprintf(f_handle, "%d:%d:%s\n", jumprecs[i]->time, jumprecs[i]->last_accessed, jumprecs[i]->path);
    }
  }

  fclose(f_handle);

  //unlock file
  fl.l_type   = F_WRLCK;
  fcntl(fileno(f_handle), F_SETLK, &fl);
  
  last_sync = now;
}

void autojump_exit()
{
  //force save.
  last_sync = 0;
  sync_to_file();
}
