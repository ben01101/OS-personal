#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

#include <fcntl.h>

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;

/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens *tokens);
int cmd_help(struct tokens *tokens);
int cmd_pwd(struct tokens *tokens);
int cmd_cd(struct tokens *tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens *tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t *fun;
  char *cmd;
  char *doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {
  {cmd_help, "?", "show this help menu"},
  {cmd_exit, "exit", "exit the command shell"},
  {cmd_pwd, "pwd", "print working directory"},
  {cmd_cd, "cd", "change directory"},
};


/* Prints a helpful description for the given command */
int cmd_help(struct tokens *tokens) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(struct tokens *tokens) {
  exit(0);
}

/* print current working directory */
int cmd_pwd(struct tokens *tokens) {
  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) != NULL)
       printf("Current working dir: %s\n", cwd);
   else
       perror("getcwd() error");
  return 1;
}

/* Change current working directory */
int cmd_cd(struct tokens *tokens) {
  char *newDir = tokens_get_token(tokens, 1);
  int numTokens = tokens_get_length(tokens);
  if (numTokens != 2) {
    fprintf(stderr, "Invalid usage: %s\n", tokens_get_token(tokens, 0));
    return -1;
  }
  if (newDir) {
    if (chdir(newDir) == -1) {
      printf("Failed to change directory: %s\n", strerror(errno));
      // cmd_pwd(tokens);
      return -1;  /* No use continuing */
    } else { 
      cmd_pwd(tokens);
      return 0;
    }
  } else {
    fprintf(stderr, "Invalid usage: %s\n", tokens_get_token(tokens, 0));
    return -1;
  }
}

/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

/* Swap character d for character e in string str. Preserves str. Returns
a new string newStr. */
void charSwap(char* newStr, char* str, char d, char e) {
  for (int i = 0; ; i++) {
    if (str[i] == '\0') {
      break;
    }
    if (str[i] == d) {
      newStr[i] = e;
      continue;
    } 
    newStr[i] = str[i];
  }
  return;
}

/* Extract program name and args and filename to read from or write to. */
int program_Parse(struct tokens *tokens, int numTokens, char *process[], 
          char *fileIn, char *fileOut, bool *redirect, bool *feed) {
  int j = 0;
  // int numArgs = tokens_get_length(tokens);
  for (int i = 0; i < numTokens; i++) {
    char* arg = tokens_get_token(tokens, i);
    int file = 0;
    if (*arg == '>') {
      file = 1;
      // j = i + 1;
      // process[i] = NULL;
      // continue;
    }
    if (*arg == '<') {
      file = 2;
      // j = i + 1;
      // process[i] = NULL;
      // continue;
    }
    if (file) {
      if (file == 1) {
        if (*redirect) {
          return 0;
        }
        *redirect = true;
        process[i] = NULL;
        i++;
        arg = tokens_get_token(tokens, i);        
        strcpy(fileOut, arg);
      } else {
        if (*feed) {
          return 0;          
        }
        *feed = true;
        process[i] = NULL;
        i++;
        arg = tokens_get_token(tokens, i);        
        strcpy(fileIn, arg);        
      }
      // process[i] = NULL;
    } else {
      if (*redirect || *feed) {
        return 0;                  
      }
      process[i] = arg;
    }
  }
process[numTokens] = NULL;
// file[1] = NULL;
return 1;
}

int main(int argc, char *argv[]) {
  init_shell();

  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);

  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    struct tokens *tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
      /* REPLACE this to run commands as programs. */
      // shell_pgid = getpid();
      bool redirect = false, feed = false;
      int numTokens = tokens_get_length(tokens);
      char *process[numTokens + 1];
      char *fileIn = (char*) malloc(1024 * sizeof(char));
      char *fileOut = (char*) malloc(1024 * sizeof(char));
      // file[0] = NULL;
      pid_t pid;


      if (!program_Parse(tokens, numTokens, process, fileIn, fileOut, &redirect, &feed)) {
        fprintf(stderr, "Invalid usage.\n");
        fprintf(stdout, "%d: ", ++line_num);
        tokens_destroy(tokens);
        continue;
      }        
      printf("redirect: %d\n", redirect);
      printf("feed: %d\n", feed);
      printf("process: [");
      for (int i = 0; process[i] != NULL; i++) {
        printf("%s, ", process[i]);
      }
      printf("]\n");
      
      printf("fileIn: %s\n", fileIn);
      printf("fileOut: %s\n", fileOut);

      int fileError = 0;
      int fromFile, toFile;
      int tempfd, tempfd2;
      if (redirect) {
        if ((toFile = open(fileOut, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0){
          fprintf(stderr, "File not found: \"%s\"\n", fileOut);
          fileError = 1;
        } else {
          tempfd = dup(1);
          dup2(toFile, 1);
        }
        // if (feed) { //Redirect and Feed.
        //   if ((fromFile = open(fileOut, O_RDONLY)) < 0){
        //     fprintf(stderr, "File not found: \"%s\"\n", fileOut);
        //     fileError = 1;
        //   } else {
        //     tempfd2 = dup(0);
        //     dup2(fromFile, 0);
        //   }
        // }
      }
      if (feed) {
        if ((fromFile = open(fileIn, O_RDONLY)) < 0){
          fprintf(stderr, "File not found: \"%s\"\n", fileIn);
          fileError = 1;
        } else {
          tempfd2 = dup(0);
          dup2(fromFile, 0);
        }
      }
      if (!fileError) {
        switch ( (pid = fork()) ) {
          case -1:
            /* Fork() has failed */
            perror ("fork");
            break;
          case 0:
            /* This is processed by the child */
            ;
            // printf("child says\n");
            char* env = getenv("PATH");
            // char* env = "blah:bleh:blue";
            char envList[4096];
            charSwap(envList, env, ':', ' ');

            struct tokens *envTok = tokenize(envList);
            for (int i = 0; i < (int) tokens_get_length(envTok); i++) {
              char tmp[200];
              strcpy(tmp, tokens_get_token(envTok, i));
              strcat(tmp, "/");
              strcat(tmp, process[0]);
              // printf("looking.. %s\n", tmp);
              execv(tmp, process);
            }
            // execv (process[0], process);
            printf("Uh oh! %s\n", strerror(errno));
            exit(EXIT_FAILURE);
            break;
          default:
            /* This is processed by the parent */
            wait(NULL);
            break;

        }
        if (redirect) {
          dup2(tempfd, 1);
          close(toFile);
        }
        if (feed) {
          dup2(tempfd2, 0);
          close(fromFile);
        }
        // End of parent program
      }
    }
      // fprintf(stdout, "This shell doesn't know how to run programs.\n");

    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);

    /* Clean up memory */
    tokens_destroy(tokens);
  }

  return 0;
}
