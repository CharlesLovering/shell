//header file for shell.cpp

#ifndef SHELL
#define SHELL

//function prototype
int execArgs(char* argv[], bool background, struct job* listofjobs[]);
char* prompt();
int shell(struct job *listofjobs[]);
int process(char* result, job *listofjobs[]);
char* readline();
void displayData(struct timeval begin, struct timeval end, struct rusage ru);
int capture(int pid, int index, struct job *listofjobs[], bool background);
int incrementIndex();
int getIndex();
void jobs(int index, struct job *listofjobs[]);
void jobSetUp(struct job *listofjobs[]);
int addToList(struct job *new_job, struct job *listofjobs[], int index);
void removeFromList(job *finished_job, job *listofjobs[]);
struct job* findFinishedJob(int fpid, struct job* listofjobs[]);
char * convertCommands(char* commands[]);

//struct job
struct job{
  int pid;
  char* command;
  std::string status;
};

//constants
#define MAX_JOBS 100
#define MAX_ARGS 32
#define MAX_BUF 129

#endif
