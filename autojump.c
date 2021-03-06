//Written by Phillp Taylor, Dan Kendall.
//Licensed under the GPL Version 2.

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/file.h>
#include <errno.h>
#include <regex.h>

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

//these functions are all used by sync_to_file
int lock_file(FILE *f_handle);
int unlock_file(FILE *f_handle);
int load_file(FILE *f_handle, struct dirspec **merge_array);
void merge_into_jumprecs(struct dirspec **merge_array);
void write_file(FILE *f_handle);

//these functions are used in the scoring system to
//choose the best match for a record or the lowest
//match when we want to eliminate unused directories.
unsigned int get_score(struct dirspec *array[], int i);
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

int autojump_initialised = 0;

void autojump_init()
{
  int i;

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
    jumprecs[i] = NULL;

  entered = 0;
  last_sync = 0;
  current_directory = NULL;

  sync_to_file();

  autojump_initialised = 1;
}


int get_best_score(struct dirspec *array[], int size)
{
  int best = -1;
  int i;

  for (i = 0; i < size; i++)
  {
    if (array[i] != NULL)
    {
      if (best == -1)
        best = i;
      else if (get_score(array, i) > get_score(array, best))
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

  if (autojump_initialised == 0)
    autojump_init();

  //sync if need be.
  sync_to_file();

  now = time(NULL);

  //update the score for the directory we just left.
  if (current_directory != NULL)
  {
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

    free(current_directory);
  }

  //note that we are now in a new directory
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
      if (get_score(jumprecs, lowest_scoring_item) > get_score(jumprecs, i)
		&& strcmp(jumprecs[i]->path, current_directory) != 0)
        lowest_scoring_item = i;
    }

    free(jumprecs[lowest_scoring_item]->path);
    jumprecs[lowest_scoring_item]->path = (char*) malloc ((sizeof(char) * strlen(new_path)) + 1);
    strcpy(jumprecs[lowest_scoring_item]->path, new_path);
    jumprecs[lowest_scoring_item]->time = 0;
    jumprecs[lowest_scoring_item]->last_accessed = now;

  }

}

unsigned int get_score(struct dirspec *array[], int i)
{
  if (array[i] == NULL)
    return -1;

  //scale last_accessed down to "days". we don't someone to
  //go on holiday for a week to find out that new directories
  //crush old directories in last_accessed scores.
  return ((array[i]->last_accessed % 3600 % 24 % 365) + (array[i]->time % 120));

}

char *autojump_jump(char *criteria)
{

  int i;
  int match;
  int best_match = -1;
  regex_t pattern;

  if (criteria == NULL && last_criteria == NULL)
    return;
  else if (criteria != NULL && last_criteria != NULL)
  {
    free(last_criteria);
    last_criteria = (char*) malloc ((sizeof(char) * strlen(criteria)) + 1);
    strcpy(last_criteria, criteria);
  }
  else if (criteria == NULL && last_criteria != NULL)
  {
    criteria = last_criteria;
    printf("criteria: %s\n", criteria);
  }

  if (regcomp(&pattern, criteria, REG_EXTENDED|REG_NOSUB|REG_ICASE) != 0)
  {
    printf("bad regex specification\n");
    if (last_criteria != NULL)
    {
      free(last_criteria);
      last_criteria = NULL;
      return NULL;
    }
  }

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
       //ignore the directory you're in, you obviously don't want to jump there.
       if (strcmp(jumprecs[i]->path, current_directory) != 0)
       {
         //match each record, if it is a match, mark it if it has the
         //best score.
         if (regexec(&pattern, jumprecs[i]->path, (size_t)0, NULL, 0) == 0)
         {
           if (best_match == -1)
              best_match = i;
           else
           {
              if (get_score(jumprecs, i) > get_score(jumprecs, best_match))
              {
                best_match = i;
              }
           }
         }
       }
    }
  }

  if (best_match == -1)
    return NULL;
  else
    return jumprecs[best_match]->path;
}

void autojump_jumpstat()
{
  int i;
  unsigned int score;
  unsigned int total = 0;

  sync_to_file();

  printf("scores:\n");

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
      score = get_score(jumprecs, i);
      printf("score: %d\t: %s\n", score, jumprecs[i]->path);
      total += score;
    }
  }

  printf("Total: %d\n", total);
}

/*
 * read_failed reason:
 * 1: fopen failed
 * 2: bad deliminator - wax file
 * 3: strtol fail - wax file
*/

//TODO: better recovery from error.
void sync_to_file()
{
  int now;
  char *temp;
  int i;
  char filename[512];
  int read_failed = 0;
  struct dirspec **merge_array;
  FILE *f_handle;

  //step one, check we need to actually sync
  now = time(NULL);

  if ((now - last_sync) < AUTOJUMP_SYNC_TIME_SECONDS)
    return;

  temp = getenv("HOME");
  strcpy(filename, temp);
  strcat(filename, AUTOJUMP_FILENAME);

  //open for reading and writing
  f_handle = fopen(filename, "r+");
  if (f_handle == NULL)
  {
    srand(time(NULL));
    last_sync += (rand() % 3600);

    //file doesn't exist, create.
    f_handle = fopen(filename, "w");
    fclose(f_handle);
    return; 
  }
  else
  {
    if (lock_file(f_handle) == -1)
    {
      srand(time(NULL));
      last_sync += (rand() % 3600);
      fclose(f_handle);
      return; 
    }
    else
    {

      //prepare the merge_array (allocating pointers, not objects)
      merge_array = (struct dirspec**) malloc(sizeof(struct dirspec*) * (AUTOJUMP_DIR_SIZE * 2));

      for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
         merge_array[i] = NULL;

      i = load_file(f_handle, merge_array);
      if (i == 0)
        merge_into_jumprecs(merge_array);

      write_file(f_handle);
      unlock_file(f_handle);
      fclose(f_handle);
      last_sync = now;
    }
  }
}

int lock_file(FILE *f_handle)
{
  struct flock fl;

  fl.l_type   = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 1;
  fl.l_pid    = getpid();

  return fcntl(fileno(f_handle), F_SETLK, &fl);
}

int unlock_file(FILE *f_handle)
{
  struct flock fl;

  fl.l_type   = F_UNLCK;
  fl.l_whence = SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 1;
  fl.l_pid    = getpid();

  return fcntl(fileno(f_handle), F_SETLK, &fl);
}

int load_file(FILE *f_handle, struct dirspec **merge_array)
{
  char *buffer;
  unsigned int total_time = 0;
  unsigned int last_accessed = 0;
  char *temp;
  int lines_read = 0;
  int read_error = 0;
  int next_free = 0;
  char *delim_one, *delim_two;
  int i;


  buffer = (char*) malloc (sizeof(char) * MAX_LINE_SIZE);

  while (fgets(buffer, MAX_LINE_SIZE - 1, f_handle) != NULL && read_error == 0)
  {
    *(buffer + (strlen(buffer) - 1)) = '\0';

    //parse out the time.
    delim_one = strstr(buffer, ":");
    if (delim_one == NULL)
    {
      read_error = 2;
      break;
    }

    //parse out the last_accessed value
    delim_two = strstr(delim_one + 1, ":");
    if (delim_two == NULL)
    {
      read_error = 2;
      break;
    }

    errno = 0;
    total_time = strtol(buffer, &delim_one, 10);
    last_accessed = strtol(delim_one + 1, &delim_two, 10);
    if (errno != 0)
    {
      read_error = 3;
      break;
    }

    //losing first char of delim_two so don't need to allocate extra for \0
    temp = (char*) malloc(sizeof(char) * strlen(delim_two));
    strcpy(temp, delim_two + 1);

    //read a whole line succesfully, add it to the array
    merge_array[next_free] = (struct dirspec*) malloc (sizeof(struct dirspec));
    merge_array[next_free]->path = temp;
    merge_array[next_free]->time = total_time;
    merge_array[next_free]->last_accessed = last_accessed;

    next_free++;

  }

  //set remaining values to NULL
  while (next_free < (AUTOJUMP_DIR_SIZE * 2))
  {
    *(merge_array + next_free) = NULL;
   next_free++;
  }

  free(buffer);

  if (read_error != 0)
    printf("AJ read error: %d\n", read_error);

  return read_error;
}

void merge_into_jumprecs(struct dirspec **merge_array)
{
  int i, put;

  //step 1, take data from jumprecs to merge array
  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {

    if (jumprecs[i] != NULL)
    {
      put = find_match(merge_array, jumprecs[i]->path);
      if (merge_array[put] != NULL)
      {
	//merge records
        if (merge_array[put]->time < jumprecs[i]->time)
          merge_array[put]->time = jumprecs[i]->time;

        if (merge_array[put]->last_accessed < jumprecs[i]->last_accessed)
          merge_array[put]->last_accessed = jumprecs[i]->last_accessed;

        jumprecs[i] = NULL;
      }
      else
      { //move entry to free space.

        merge_array[put] = jumprecs[i];
	jumprecs[i] = NULL;

      }
    }
  }

  //step 2, limited positions up for grabs!
  for (i = 0; i < (AUTOJUMP_DIR_SIZE); i++)
  {
    i = get_best_score(merge_array, (AUTOJUMP_DIR_SIZE * 2));
    if (i == -1)
      break;
    else
    {
      jumprecs[i] = merge_array[i];
      merge_array[i] = NULL;
    }
  }

  //step 3, the losers get deleted.
  for (i = 0; i < (AUTOJUMP_DIR_SIZE * 2); i++)
  {
    if (merge_array[i] != NULL)
    {
       free(merge_array[i]->path);
       free(merge_array[i]);
    }
  }

}

int find_match(struct dirspec **merge_array, char *path)
{
  int i;

  for (i = 0; i < (AUTOJUMP_DIR_SIZE * 2); i++)
  {

    if (merge_array[i] != NULL)
    {
      if (strcmp(merge_array[i]->path, path) == 0)
         return i;
    }
    else
      return i;
  }
}

void write_file(FILE *f_handle)
{

  int i;

  rewind(f_handle);

  for (i = 0; i < AUTOJUMP_DIR_SIZE; i++)
  {
    if (jumprecs[i] != NULL)
    {
       fprintf(f_handle, "%i:%i:%s\n", jumprecs[i]->time, jumprecs[i]->last_accessed, jumprecs[i]->path);
    }
  }

}

void autojump_exit()
{
  //force save.
  last_sync = AUTOJUMP_SYNC_TIME_SECONDS + 25;
  sync_to_file();
}
