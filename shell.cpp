#include <string.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include "shell.h"


using namespace std;

int CURRENT_INDEX = -1;

int main(int args, char* argv[]) {

  int loop = 0; 
  struct job *listofjobs[MAX_JOBS];

  if (args > 1){
    execArgs(&argv[1], false, listofjobs);
  } else {
    while (loop >= 0){
      loop = shell(listofjobs);
    }
  }
  return 0;
}

/*
 * Calls prompt and process to emulate a shell
 */
int shell(struct job *listofjobs[]){
  char* result = prompt();
  int val = process(result, listofjobs);
  return val;
}

char* prompt(){
  //shell display
  cout << ">>> ";

  //reads a line to be executed - waiting for it...
  char* input = readline();
 
  //if nothing was inputted, prompt again.
  if((input[0] == '\n')|(strlen(input) == 0)){
    input = prompt();
  }
  return input;
}

/*
 * Reads line from stdin, and removes \n 
 */
char* readline() {
  char *line = NULL;
  size_t bufsize = MAX_BUF; //getline controls buffer
  getline(&line, &bufsize, stdin);
  int i = 0;
  while (line[i]){
    if (line[i] == '\n'){
      line[i] = '\0';
    }
    i++;
  }
  return line;
}

/*
 * Manages user input and calls execArgs to execute commands
 */
int process(char* result, struct job *listofjobs[]){
  //formatting input into a char* array
  char* command[32];
  int size = 0;
  char* input = strtok(result, " ");

  while (input != NULL){
    command[size] = input;
    input = strtok(NULL, " \t\r\n");
    size++;
  }
  command[size] = NULL;

  if((size == 1)&&(strstr(command[0], "exit")) != NULL){
    return -1;
    //exit(0);
  } else if ((size == 2) && (strstr(command[0], "cd") != NULL)){
    char* path = command[1];
    int i = chdir(path);
    wait(0);
    if (i != 0){
      cout << "FAILED TO CHANGE DIR" << endl;
    }
  } else if (strstr(command[0], "jobs") != NULL){
      jobs(getIndex(), listofjobs);
  } else if(strstr(command[size - 1], "&") != NULL){
      command[size - 1] = NULL;
      execArgs(command, true, listofjobs);
  } else {
    execArgs(command, false, listofjobs);
  }
  return 0;
}



/*
 * displays all jobs (pids and commands)
 */
void jobs(int index, struct job *listofjobs[]){

  cout << "This method was not fully functional. \nThe problem is that processes" <<
          "do not share memory. This makes it difficult to maintain/coordinate a global index (never mind an array of jobs.)\n" <<
          "It would have been work-aroundable - but to make background tasks I created a seperate listener process that waited for the\n" <<
          "the background process. In order to do this, and use getrusage to get information, I have to fork 'above' the executing\n" <<
          "background process. Thus, under my current structure, getting usage data and job data is an either-or-situation.\n" <<
          "To still provide the information, the built-in ps command will now execute.\n" << endl;

  int pid = fork();
  
  if (pid < 0){
    cerr << "error forking" << endl;
  }

  if (pid == 0){
    char* argv[2];
    argv[0] = "ps";
    argv[1] = NULL;
    execvp(argv[0], argv);
  }

  if (pid > 0){
    wait(0);
    cout << ">>>";
    return;
  }

  /*
  if (index < 0){
    cout << "There are no jobs currently running." << endl;
    return;
  }

  if (index >= MAX_JOBS){
    cout << "Too many jobs!" << endl;
    return;
  }
  
  for (int i = 0; i < index + 1; i++){
    cout <<  listofjobs[i] << endl;
    //cout << listofjobs[i] -> pid << "\t\t\t\t" << listofjobs[i] -> command << listofjobs[i] -> status << endl;
  }
  */
}



int execArgs(char* argv[], bool background, struct job* listofjobs[]){
  int index = getIndex(); //sets and gets the next free index if background

  //if background and too many jobs were run, exit.
  if ((index + 1 >= MAX_JOBS) && (background)) {
      cout << "Maximum number of jobs reached. Please wait and try again." << endl;
      exit(0);
    } else {  
      index = incrementIndex(); //increment the number of jobs
  }

  //forking
  int pid = fork();

  //check for error                                                                                    
  if (pid < 0){
    cerr << "There was an error in fork" << endl;
    exit(1);
  }

  //in child
  if (pid == 0){
    
    //if not in background, exec command
    if (!background) {
      int rv = execvp(argv[0], &argv[0]);
      if (rv < 0){
        cerr << "Error executing execvp" << endl;
        exit(1);
      }
    }

    //if a background task:
    if (background){

      //fork - to create a listener process 
      //NOTE: the problem here is that the below pid value is the value that I 'need' for jobs()
      //and there is no way to share this with the 'grand-parent' process - that I could find, without
      //using something like mmap, which i found required libraries that the ccc did not have.
      //clearly I missed something - but that being said, everything but jobs works correctly!
      int pid_background_child = fork();

      //check for fork error
      if (pid_background_child < 0){
        cerr << "Error forking" << endl;
        exit(1);
      }

      //this is the listener process
      if (pid_background_child > 0){

        //add the job to the list of jobs
        struct job *current_job = new job;
        char* buffer = (char*)(malloc(sizeof(char) * 129));
        strcat(buffer, convertCommands(argv));
        current_job -> command = buffer;
        current_job -> status = "Running";
        current_job -> pid = pid_background_child;
        addToList(current_job, listofjobs, index);
    
        //wait for the background process to finish in capture
        capture(pid_background_child, index, listofjobs, true);
        exit(0);
      }

      //we'll execute the background process in this child
      if (pid_background_child == 0){
        int exec = execvp(argv[0], &argv[0]);
        if (exec < 0) {
          cerr << "Error executing execvp" << endl;
          exit(1);
        }
      }
    }
  } 
  //in parent
  if (pid > 0) {
    if (!background){
        //wait for the command to finish in capture
        capture(pid, -1, listofjobs, false);
        return 0;
    } else if (background) {
      //running command in the background
      return 0;
    }
  }
  //'unreachable'
  return -127;
}

/*
 * captures/catches/waits from the process to end.
 * waits and then prints out the data.
 *
 */
int capture(int pid, int index, struct job *listofjobs[], bool background){

  struct rusage ru;
  struct timeval first, end;
  int status;

  gettimeofday(&first, NULL);
  waitpid(pid, &status, 0);
  gettimeofday(&end, NULL);
  getrusage(RUSAGE_CHILDREN, &ru);

  if (background){
    //notify user that background process is complete
    listofjobs[index] -> status = "Done";
    cout << "\nBACKGROUND PROCESS FINISHED:" << endl;
    cout << "\nPID:  " << listofjobs[index] -> pid << "\tCommand:  " << listofjobs[index] -> command << "\tStatus:  " << listofjobs[index] -> status << endl;
    displayData(first, end, ru);
    cout << ">>>";
  } else {
    displayData(first, end, ru); 
  }

  return 0;
}


/*
 * displays the usage statistic data of the finished process
 */
void displayData(struct timeval begin, struct timeval end, struct rusage ru){
  printf("\n");  
  printf("\n\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
  printf("\n\nUSAGE STATISTICS:\n\n");
  printf("User Time: %ld [ms]\n", (ru.ru_utime.tv_sec * 1000 + (ru.ru_utime.tv_usec / 1000)));
  printf("System Time: %ld [ms]\n", (ru.ru_stime.tv_sec * 1000 + (ru.ru_stime.tv_usec / 1000)));
  printf("WallClockTime: %ld [ms]\n", ((end.tv_sec - begin.tv_sec) * 1000) + ((end.tv_usec - begin.tv_usec) / 1000));
  printf("Major Page Faults: %ld\n", ru.ru_majflt);
  printf("Minor Page Faults: %ld\n", ru.ru_minflt);
  printf("Voluntary Context Switches: %ld\n", ru.ru_nvcsw);
  printf("Involuntary Context Switches:  %ld\n\n", ru.ru_nivcsw);
  printf("\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n");
  printf("\n");
}

/*
 * In order to minimalize how bad global variables are, going to use these 'getters and setters' for index,
 * These ended up not really helping that much since memory is not shared between processes
 */
 int incrementIndex(){
  CURRENT_INDEX = CURRENT_INDEX + 1;
  return CURRENT_INDEX;
}
  int getIndex(){
  return CURRENT_INDEX;
}

/*
 * Adds a new job to the list of jobs, finding the first
 * empty slot in the array of jobs.
 */
int addToList(job *new_job, job *listofjobs[], int index){
  //delete(&listofjobs[index]);
  listofjobs[index] = new_job;
  return 0;
}

/*
 * Converts a *char[] to a char*
 */
char* convertCommands(char* commands[]){
  //string *command = new string;

  char *command = (char*)(malloc(sizeof(char) * 129));


  int i = 0;
  while(commands[i] != NULL){
    strcat(command, commands[i]);
    strcat(command, " ");
    i++;
  }

  return command;
}
