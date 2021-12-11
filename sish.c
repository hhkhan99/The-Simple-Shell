#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

// function Prototype 
int processString(char *str);
void add_command_to_history( const char *command );

const int HISTORY_MAX_SIZE = 100;

unsigned history_count = 0;
int batchMode=0;

//Function to display the prompt "sish> " and get users input
int displayPrompt(void) 
{
      char buf[100];
      char *cmds[100];
      char *tmp;
      int noOf_cmds, i, flag, rc=0;

      while (!rc) 
      {   
          noOf_cmds = 0;
          printf("\n./sish>> "); fgets(buf, 512, stdin);

          for(i=0; buf[i]!='\0'; i++) {
            if(buf[i]=='+')
              flag++;
          }

          int j = i-2;
          int endingWithPlus = 0;
          for(; j >= 0; j--)
          {
            if(buf[j] == ' ')
              continue;
            else if(buf[j] == '+')
              {
                endingWithPlus = 1;
                break;
              }
            else
              break;
          }
          

          //for tokenizing the string in buf
          tmp = strtok(buf,"+");
           
          while (tmp) 
          {
              cmds[noOf_cmds] = tmp;
              noOf_cmds++;
              tmp = strtok(NULL,"+");
          } 
          // store the total commands to use it to fork 
          int noOfCmndsToFork = noOf_cmds;
          
          
          if(flag==0)
          {
            if((rc=processString(buf))==101) {
              break; 
              exit(1);
            }
          }
          else
            {
              
              if(endingWithPlus == 0)
              {
                  noOfCmndsToFork = noOf_cmds - 1;
                   if((rc=processString(cmds[noOf_cmds-1]))==101)
                   {
                     
                     break; exit(1);
                   }
              }

              int i;
              for (i = 0; i < noOfCmndsToFork; i++) 
                {
            
                  int ret;
                  if( (ret=fork()) > 0 )
                    {
                      //   parent(ret);
                    }
                  else if ( ret == 0 )
                    {
                      //    child();
                      
                      if(processString(cmds[i])==101) 
                        { 
                          
                          exit(0); 
                        }
                      break;
                    }
                  else 
                    {
                      char error_message[30] = "error occurred\n";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                    }
                }
            }
      }
      return 0;
}



// function for parsing command words and implementing built in commands
int breakCmd(char *str)
{
       char *tmp;
       char *subcmds[1000];
       char buf[1000];
       strcpy(buf, str);
       tmp = strtok(buf," \n\t");
       int num_subcmds = 0;
       int out, flag1=0;
       char *subnew[1000];
        
       while (tmp) 
       {
          subcmds[num_subcmds] = tmp;
          num_subcmds++;
          tmp = strtok(NULL," \n\t");
       } 
            
       int j, loc=0;

      for (j = 0; j < num_subcmds; j++) 
         {
         }

       subcmds[j] = NULL;

       if(num_subcmds > 1)
         {
           for (j = 0; j < num_subcmds; j++) 
             {
               if(strcmp(subcmds[j], ">") == 0)
                 {
                   loc=j;
                   flag1=1;
                   break;
                 }
             }

           

           if(flag1==1)
             {
               for (j = 0; j < loc; j++) 
                 {
                   subnew[j]=subcmds[j];
                 }
             }
       
           subnew[loc]=NULL;
         }
       
       int i, savedJ, flag2 = 0;

       if(flag1!=1)
           for (j = 0; j < num_subcmds; j++) 
             {
               i = strlen(subcmds[j]) - 1;
               
               if(subcmds[j][i]=='>')
                 {
                   subcmds[j][i]='\0';
                   flag2 = 1;
                   savedJ=j;
                 }
             }

       if(flag2==1)
         {
           for (j = 0; j <= savedJ; j++) 
             {
               subnew[j]=subcmds[j];
             }
           subnew[savedJ+1]=NULL;
         }
 
       // pipe cmds implementation
       
       if(flag1==1)
         {
           close(STDOUT_FILENO);
           out = open(subcmds[loc+1], O_RDWR | O_CREAT , 0666 ); 
             
           if(out < 0)
             {
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(0);
               
             }
           dup2(out, STDOUT_FILENO);
           if(execvp(subnew[0], subnew) < 0)
             {
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(101);
             }
           close(out);
           return 101;
         }
       else if(flag2==1)
         {
           close(STDOUT_FILENO);
           out = open(subcmds[savedJ+1], O_RDWR | O_CREAT , 0666 ); 
          
           if(out < 0)
             {
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(0);
               
             }
           dup2(out, STDOUT_FILENO);
           if(execvp(subnew[0], subnew) < 0)
             {
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(101);
             }
           close(out);
           return 101;
         }
         // for cd command 
         else if(strcmp(subcmds[0], "cd") == 0)
         {
           // if the command after cd exists and directory then change directory
           int res;
           if(subcmds[1]!=NULL)
             {
               res = chdir(subcmds[1]);
             }
            // else go to home
           else
             {
               res = chdir(getenv("HOME"));
             }
           
           if(res == -1)
             {
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(101);
               
             }
         }
         //when called exit
       else if(strcmp(subcmds[0], "exit") == 0)
         {
           
           exit(0);
           
         }
         // to display current wqorking directory
       else if(strcmp(subcmds[0], "pwd") == 0)
         {
           if(subcmds[1]!=NULL)
             {
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(0);
               
             }
           else if (execvp(subcmds[0], subcmds) < 0)
             {
               
               char error_message[30] = "error occurred\n";
               write(STDERR_FILENO, error_message, strlen(error_message));
               exit(101); 
             }
         }
       else if (execvp(subcmds[0], subcmds) < 0)
         {
           
           char error_message[30] = "error occurred\n";
           write(STDERR_FILENO, error_message, strlen(error_message));
           
           return 1;
         }
       //exit
       return 1;
}

int processString(char *str)
{
       char *tmp;
       char *subcmds[1000];
       char buf[1000];
       strcpy(buf, str);
       tmp = strtok(buf,";");
       int num_subcmds = 0;
        
       while (tmp) 
       {
          subcmds[num_subcmds] = tmp;
          num_subcmds++;
          tmp = strtok(NULL,";");
       } 

       int j, status;
           
       for (j = 0; j < num_subcmds; j++) 
       {
         
         int ret;
         if((subcmds[j][0]=='c' && subcmds[j][1]=='d') == 1)
           {
             breakCmd(subcmds[j]);
           }
         else if(strcmp(subcmds[j],"exit") == 0)
           {
             breakCmd(subcmds[j]);
           }
         else
           {
             if( (ret=fork()) > 0 )
               {
                 //parent(ret);
                 //wait(&status);
                 waitpid(ret,&status,WUNTRACED);
                 int x = WEXITSTATUS(status);
            
                 if(x==101)
                   return 101;
                   
               }
             else if ( ret == 0 )
               {
                 //child();
                 
                 if(breakCmd(subcmds[j])==1)  
                   { 
                      
                     exit(1);
                     return 101; 
                   }
                 else return 0;
                 break;
               }
             else 
               {
                 char error_message[30] = "error occurred\n";
                 write(STDERR_FILENO, error_message, strlen(error_message));
                 exit(101);
               }
           }
       }
      return 0;
}

//main function

int main(int argc, char *argv[]) 
{
      char *cmds[1000];
      char buf[1000]="test";
      char * tmp;
      int noOf_cmds, i, flag, rc=0;
      //int batchMode=0;
      char *fileToRead = "/no/such/file";

      if(argc>2) 
        {
          char error_message[30] = "error occurred\n";
          write(STDERR_FILENO, error_message, strlen(error_message));
          exit(1);
        }
      else if(argc==2)
        {
          batchMode = 1;                
          fileToRead = argv[1];
        }

      FILE *new;
      FILE *fp;

      if(batchMode==1)
        {
          fp = fopen(fileToRead,"r");
          if (fp==NULL) 
            {
              char error_message[30] = "error occurred\n";
              write(STDERR_FILENO, error_message, strlen(error_message));
              exit(1);
            }
          new=fp;
        }
      else
        new=stdin;
        
      while (!feof(new)) 
      {
          noOf_cmds = 0;
          if(batchMode==0) write(STDOUT_FILENO, "sish> ", strlen("sish> ")); 
            
          if(batchMode==1)
              fgets(buf, 1000, fp);
          else
              fgets(buf, 1000, stdin);

          int j;

          strtok(buf, "\n\r");

          if(batchMode == 1 && strcmp(buf,"xyz")!=0)
            {
                 
                  write(STDOUT_FILENO, buf, strlen(buf));
                  write(STDOUT_FILENO, "\n", strlen("\n"));
                  
                  if(strcmp(buf,"+")==0)
                    {
                      
                      exit(0);
                    }
            }

          if(strcmp(buf,"xyz")==0) exit(0);
          
         

          for(i=0; buf[i]!='\0'; i++) {
            if(buf[i]=='+')
              flag++;
          }
          
          if(strlen(buf)==0)
            {
              char error_message[30] = "error occurred\n";
              write(STDERR_FILENO, error_message, strlen(error_message));
            }

          j = i-2;
          int endingWithPlus = 0;
          for(; j >= 0; j--)
          {
            if(buf[j] == ' ')
              continue;
            else if(buf[j] == '+')
              {
                endingWithPlus = 1;
                break;
              }
            else
              break;
          }

          
          tmp = strtok(buf,"+");
           
          while (tmp) 
          {
              cmds[noOf_cmds] = tmp;
              noOf_cmds++;
              tmp = strtok(NULL,"+");
          } 

          int noOfCmndsToFork = noOf_cmds;
          
          if(flag==0)
          {
            if((rc=processString(buf))==101) {
              
              break; exit(0);
            }
          }
          else
            {
              
              if(endingWithPlus == 0)
              {
                  noOfCmndsToFork = noOf_cmds - 1;
                   if((rc=processString(cmds[noOf_cmds-1]))==101)
                   {
                     
                     break; 
                     exit(0);
                   }
              }

              int i;
              for (i = noOfCmndsToFork-1; i >= 0; i--) 
                {
                 
            
                  int ret;
                  if( (ret=fork()) > 0 )
                    {
                      //   parent();
                      //printf("Hello");
                      //showPrompt();
                      //wait(&status);
                      while (1) {
                        int status;
                        pid_t done = waitpid(ret,&status,WUNTRACED);
                        if (done == -1) {
                          if (errno == ECHILD) break; // no more child processes
                        } else {
                          int x = WEXITSTATUS(status);
                        
                          if (!WIFEXITED(x) || WEXITSTATUS(x) != 101) {
                            //cerr << "pid " << done << " failed" << endl;
                            exit(0);
                          }
                        }
                      }
                    }
                  else if ( ret == 0 )
                    {
                      //    child();
                      //close(STDOUT_FILENO);
                      
                      if(processString(cmds[i])==101) 
                        { 
                          exit(0); 
                        }
                      //break;
                    }
                  else 
                    {
                      char error_message[30] = "error occurred\n";
                      write(STDERR_FILENO, error_message, strlen(error_message));
                      exit(0);
                    }
                }
            }
          strcpy(buf,"xyz");
      }
      return 0;
}