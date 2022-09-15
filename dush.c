#include<time.h>
#include<stdbool.h>
#include<string.h>
#include<assert.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>

void main_loop();
char** separate_command(char* buffer, int* words);
bool is_builtin(char** command_words, int words, char** path);
void change_directory(char* location);
void overwrite_path(char** command_words, int words, char** path);

int main() {
  main_loop();
  return 0;
}
void main_loop(){
  char* buffer;
  char** command_words;
  int words = 1;
  size_t bufsize = 128;
  size_t input;

  char* path[32] = { "\0" };
  path[0] = "/bin";

  do {
    words = 1;
    printf("dush> ");
    input = getline(&buffer, &bufsize, stdin);
    command_words = separate_command(buffer, &words);

    if (command_words != NULL) {
      if (!is_builtin(command_words, words, path)) {
        printf("Non built-in command!\n");
      }
    }

  } while ( /*strcmp(buffer, "exit\n")*/ true);

  // remember to free this string array because
  // its memory was allocated on the heap.
  free(command_words);

  exit(0);

}

char** separate_command(char* buffer, int* words){
  /*
    This function takes in a string command and separates it into
    a string array where each index is an individual word. This
    function automatically deals with repeated whitespace.

    Input:
      char* buffer : string command
    Output:
      char **<name> : string array
  */

  if(strlen(buffer) <= 1) { return NULL; }
  else {
    //clearing newline char from end
    buffer[strlen(buffer)-1] = '\0';
  }

  // counting number of spaces in the string input
  char prev_char = ' ';
  for(int i = 0; i < strlen(buffer) - 1; i++){
    if (buffer[i] == ' ' && prev_char != ' ') {
       (*words)++;
    }
    prev_char = buffer[i];
  }


  // checking that the command doesn't end on emptyspace
  // checking index -2 instead of -1 b/c getline adds
  // \n to end of input.
  if (buffer[strlen(buffer) - 2] == ' ') { (*words)--; }

  // Dynamically allocating memory for the string
  // array because we want to return this data, and
  // we cannot return values from the stack (where
  // data is normally stored without using malloc.
  // memset for example stores on the stack).
  char ** command_words = malloc((*words) * sizeof(char*));
  for (int i = 0 ; i < (*words); ++i) {
        // Setting 32 character limit for a single command
        // word... hopefuly that's enough?
        command_words[i] = malloc(32 * sizeof(char));
  }

  // Populating the string array
  char* found;
  int i = 0;
  while(i < (*words)){
    found = strsep(&buffer, " ");
    if (strcmp(found,"\0") != 0) {
      strcpy(command_words[i], found);
      i++;
    }
  }

  return command_words;

}

bool is_builtin(char** command_words, int words, char** path){

  char* command = command_words[0];

  if (strcmp(command, "exit") == 0 && words == 1){
    // command_words string array was dynamically allocated, so
    // don't forget to free the memory!
    free(command_words);
    exit(0);
  } else if (strcmp(command, "cd") == 0 && words == 2) {
    change_directory(command_words[1]);
    return true;
  } else if (strcmp(command, "path") == 0) {
    overwrite_path(command_words, words, path);
    return true;
  }

  return false;

}

void change_directory(char* location){
  if (chdir(location) != 0) {
    fprintf(stderr, "%s\n", strerror(errno));
  } else {
    printf("CD successful!\n");
  }
}

void overwrite_path(char** command_words, int words, char** path){
  int old_paths = sizeof(path) / sizeof(path[0]);

  for (int i = 0; i < old_paths; i++) {
    path[i] = "\0";
  }

  for (int i = 1; i < words; i++) {
    path[i-1] = command_words[i];
  }

}
