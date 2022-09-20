#include<time.h>
#include<stdbool.h>
#include<string.h>
#include<assert.h>
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/wait.h>

void main_loop(FILE * in);
char** separate_command(char* buffer, int* words);
bool is_builtin(char** command_words, int words, char** path, int* paths);
void change_directory(char* location);
void overwrite_path(char** command_words, int words, char** path, int* paths);
bool file_exists(char** command_words, int words, char** path, int paths, bool solo_command);
void run_execv(char** command_words, char* location, bool solo_command);
int contains_redirection(char** command_words, int words);
int* contains_parallel(char** command_words, int words);
char*** separate_parallel_commands(char** command_words, int words, char** path, int paths);

void dush_prompt() {
  printf("dush> ");
}

void error_prompt() {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char * argv[]) {
  FILE * input = NULL;
  if (argc == 1) {
    input = stdin;
  } else if (argc == 2) {
    char * fileName = strdup(argv[1]);
    input = fopen(fileName, "r");
    if (input == NULL) {
      error_prompt();
    }
  } else {
    error_prompt();
  }
  main_loop(input);
  return 0;
}

void main_loop(FILE * in){
  char* buffer;
  char** command_words;
  int words = 1;
  size_t bufsize = 128;
  size_t input;
  char*** separated_command_words;

  int paths = 1;
  char* path[32] = { "\0" };
  path[0] = "/bin";

  do {
    words = 1;
    dush_prompt();
    input = getline(&buffer, &bufsize, in);
    command_words = separate_command(buffer, &words);

    printf("CHECKING PARALLELS\n");
    separated_command_words == separate_parallel_commands(command_words, words, path, paths);
    printf("DONE CHECKING PARALLELS\n");

    if (command_words != NULL) {
      if (!is_builtin(command_words, words, path, &paths)) {
        bool exists = file_exists(command_words, words, path, paths, true);
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

bool is_builtin(char** command_words, int words, char** path, int* paths){

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
    overwrite_path(command_words, words, path, paths);
    return true;
  }

  return false;

}

void change_directory(char* location){
  if (chdir(location) != 0) {
    fprintf(stderr, "%s\n", strerror(errno));
  }
}

void overwrite_path(char** command_words, int words, char** path, int* paths){

  for (int i = 0; i < *paths; i++) {
    path[i] = "\0";
  }

  for (int i = 1; i < words; i++) {
    path[i-1] = command_words[i];
  }

  *paths = words - 1;

}

bool file_exists(char** command_words, int words, char** path, int paths, bool solo_command) {

  char location[64] = { '\0' };

  for (int i = 0; i < paths; i++) {
    strcpy(location, path[i]);
    strcat(location, "/");
    strcat(location, command_words[0]);
    if (access(location, X_OK) == 0) {
      run_execv(command_words, location, solo_command);
      return true;
    }
  }

  printf("%s does not exist!\n", location);
  return false;
}

void run_execv(char** command_words, char* command, bool solo_command) {
  if(fork() == 0) {
    execv(command, command_words);
    printf("Something went wrong!\n");
    exit(0);
  } else {
    wait(NULL);
    if (solo_command == false) {
      exit(0);
    }
  }
}

int contains_redirection(char** command_words, int words) {
  /*
    Returns index of a redirection char, or -1 if none found.
  */
  for (int i = 0; i < words; i++){
    if (strcmp(command_words[i], ">") == 0) {
      return i;
    }
  }
  return 0;
}

int* contains_parallel(char** command_words, int words) {
  /*
    Returns int pointer array of the indexes of command_words where parallel chars appear.
    The length is set to the number of words, but to get its size, we include the value words - 1
    after all indexes are included to mark the symbolic end of the array.
  */
  int* parallel_separators = malloc((words + 1) * sizeof(int));
  int index = 0;
  for (int i = 0; i < words; i++) {
    if (strcmp(command_words[i], "&") == 0) {
      parallel_separators[index] = i;
      index++;
    }
  }

  // setting a known end-of-list value so we may obtain its size later
  parallel_separators[index] = words-1;

  return parallel_separators;
}

char *** separate_parallel_commands(char** command_words, int words, char** path, int paths) {
  /*
    Idea of this function is to create subcommands via taking the command words between two adjacent
    indexes listed in separator_indexes. Then, within each of those we can check for redirection
    since I assume parallel commands have precedent over redirecting output.
  */

  int* separator_indexes = contains_parallel(command_words, words);
  int num_separators = 0;
  for (int i = 0; i < words; i++) {
    if (separator_indexes[i] != words - 1) { num_separators++; }
    else { break; }
  }

  /*
  // creating base array to temporarily hold the subcommands
  char** command_words_sub_command = malloc((num_separators + 1) * sizeof(char *));
  for (int i = 0; i < num_separators + 1; i++) {
    command_words_sub_command[i] = malloc(32 * sizeof(char));
  }
  */

  char*** separated_command_words = malloc((num_separators + 1) * sizeof(char **));
  for (int i = 0; i < num_separators; i++) {
    separated_command_words[i] = malloc(words * sizeof(char *));
    for (int j = 0; j < words; j++) {
      separated_command_words[i][j] = malloc(32 * sizeof(char));
    }
  }

  // separating commands to individual subcommands
  int start_index = 0;
  int end_index = 0;
  for (int i = 0; i < num_separators + 1; i++) {
    start_index = end_index;
    end_index = separator_indexes[i];
    if (i == num_separators) { end_index++; }
    for (int j = start_index; j < end_index; j++) {
      strcpy(separated_command_words[i][j - start_index], command_words[j]);
      printf("%s ", separated_command_words[i][j - start_index]);
    }
    printf("\n");
    end_index++;
  }

  //file_exists(command_words_sub_command, end_index - start_index, path, paths, false);
  return separated_command_words;

}
