/* 
 * Main source code file for lsh shell program
 *
 * You are free to add functions to this file.
 * If you want to add functions in a separate file 
 * you will need to modify Makefile to compile
 * your additional functions.
 *
 * All the best 
 */

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "parse.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * Function declarations
 */

void PrintCommand(int, Command *);
void PrintPgm(Pgm *);
void stripwhite(char *);
void signalhandler(int);
void recursion(Pgm*);

/* When non-zero, this global means the user is done using this program. */
int done = 0;
/* File descriptors used. */
int fd1;
int fd2;

/*
 * Name: main
 *
 * Description: Gets the ball rolling...
 *
 */
int main(void)
{
    int n;
    Command cmd;

    signal(SIGCHLD,SIG_IGN);
    signal(SIGINT,SIG_IGN);

    while (!done) {

        char *line;
        line = readline("> ");

        if (!line) {
            /* Encountered EOF at top level */
            done = 1;
        
        } else {
            /*
            * Remove leading and trailing whitespace from the line
            * Then, if there is anything left, add it to the history list
            * and execute it.
            */
            stripwhite(line);

            if (*line) {
            
                add_history(line);
                n = parse(line, &cmd);
                PrintCommand(n, &cmd);
                pid_t pid;
        
                if (strcmp(*cmd.pgm->pgmlist,"exit") == 0) {
                    
                    done = 1;  
                
                } else {
       
                    if (strcmp(*cmd.pgm->pgmlist,"cd") == 0) {
                    
                        if (cmd.pgm->pgmlist[1] == NULL) {
                        
                            printf("no argument\n");
                            char *home = getenv("HOME");
                            chdir(home);            
                    
                        } else {     
                        
                            char *comm = strtok(line," ");
                            printf("%s\n",comm);
                            comm = strtok(NULL," ");
                            printf("%s\n",comm);                           
                        
                            if (chdir(comm) == -1)
                                perror("chdir error:\n"); 
            
                        }            
                    } else { 
     
                        if ((pid = fork()) < 0) {
          
                            perror("Fork failed:\n");
                            exit(1);
                    
                        } else if (pid == 0) {  //  child process 
           
                            if (cmd.bakground == 1) {
             
                                signal(SIGINT,SIG_IGN);
                            }
           
                            if (cmd.rstdout != NULL) {
              
                                fd1 = open(cmd.rstdout,O_CREAT|O_WRONLY,0600);
          
                                if (fd1 == -1)
                                    perror("Cannot open file:\n");
              
                                dup2(fd1,STDOUT_FILENO);
                                close(fd1);
                            } 
                            if (cmd.rstdin != NULL) {
                        
                                fd2 = open(cmd.rstdin,O_RDONLY );
                             
                                if (fd2 == -1) {
                                    perror("Cannot read from file\n");
                                }
                             
                                dup2(fd2,STDIN_FILENO);
                                close(fd2);
                            }
               
                            recursion(cmd.pgm); 
                                                                    
                        } else {  //  parent process
          
                            signal (SIGINT,SIG_IGN);
                        
                            if (cmd.bakground == 0) {                     
             
                            wait (NULL);
                            printf("Child complete \n");
             
                            }
             
                        }        
                    }           
                }        
            }                 
        }    
    
        if(line) 
          free(line);
    
    }    
    return 0;
}

/*
 *Name:recursion
 *Description:forking and piping in order to communicate
*/


void recursion(Pgm *pgm)
{

    if (pgm->next == NULL) {
                         
        if ((execvp(*pgm->pgmlist,pgm->pgmlist)) == -1)
            perror("Execvp error in recursion:\n");   
    
    } else {
      
        pid_t pid;
        int fd[2];
      
        if (pipe(fd) == -1) {
        
            perror ("Pipe failure:\n");
            exit(1);
        
        }
        if ((pid = fork()) < 0) {
        
        perror ("Fork failure in recursion:\n");
        exit(1);
      
        } else if (pid == 0) {             
       
            dup2(fd[1], STDOUT_FILENO);     
            close(fd[0]);  
            close(fd[1]);  
            recursion(pgm->next);        
        
        } else {                    
     
            dup2(fd[0], STDIN_FILENO);      
            close(fd[1]);   
            close(fd[0]);    
            execvp(*pgm->pgmlist,pgm->pgmlist);
      
        }  
    }  
}    

/*
 * Name: PrintCommand
 *
 * Description: Prints a Command structure as returned by parse on stdout.
 *
 */
void
PrintCommand (int n, Command *cmd)
{
  printf("Parse returned %d:\n", n);
  printf("   stdin : %s\n", cmd->rstdin  ? cmd->rstdin  : "<none>" );
  printf("   stdout: %s\n", cmd->rstdout ? cmd->rstdout : "<none>" );
  printf("   bg    : %s\n", cmd->bakground ? "yes" : "no");
  PrintPgm(cmd->pgm);
}

/*
 * Name: PrintPgm
 *
 * Description: Prints a list of Pgm:s
 *
 */
void
PrintPgm (Pgm *p)
{
  if (p == NULL) {
    return;
  }
  else {
    char **pl = p->pgmlist;

    /* The list is in reversed order so print
     * it reversed to get right
     */
    PrintPgm(p->next);
    printf("    [");
    while (*pl) {
      printf("%s ", *pl++);
    }
    printf("]\n");
  }
}

/*
 * Name: stripwhite
 *
 * Description: Strip whitespace from the start and end of STRING.
 */
void
stripwhite (char *string)
{
  register int i = 0;

  while (whitespace( string[i] )) {
    i++;
  }
  
  if (i) {
    strcpy (string, string + i);
  }

  i = strlen( string ) - 1;
  while (i> 0 && whitespace (string[i])) {
    i--;
  }

  string [++i] = '\0';
}
