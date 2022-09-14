#include<time.h>
#include<stdbool.h>
#include<string.h>
#include<assert.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>

void main_loop();
char** separate_command(char* buffer, int* words);
void check_builtin(char** command_words, bool last_word);

int main() {
  main_loop();
  printf("Test!\n");
  return 0;
}
void main_loop(){
  char* buffer;
  char** command_words;
  int words = 1;
  size_t bufsize = 128;
  size_t input;

  char path[64] = "\bin";

  do {
    words = 1;
    printf("\nEnter something: ");
    input = getline(&buffer, &bufsize, stdin);
    command_words = separate_command(buffer, &words);
    check_builtin(command_words, words == 1);

  } while ( /*strcmp(buffer, "exit\n")*/ true);

  // remember to free this string array because
  // its memory was allocated on the heap.
  free(command_words);

  exit(0);
  printf("Test!\n");
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

  if(strlen(buffer) == 1) { return NULL; }

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

void check_builtin(char** command_words, bool last_word){

  char* command = command_words[0];

  //clearing any potential newline char
  //char command_no_newline[strlen(command)];
  //memset(command_no_newline, '\0', strlen(command));
  if(last_word) { command[strlen(command) - 1] = '\0'; }

  if (strcmp(command, "exit") == 0){
    // command_words string array was dynamically allocated, so
    // don't forget to free the memory!
    free(command_words);
    exit(0);
  } else if (strcmp(command, "cd") == 0) {
    printf("<Run CD command when implemented>...\n");
  } else if (strcmp(command, "path") == 0) {
    printf("<Run Path command when implemented>...\n");
  }

}
