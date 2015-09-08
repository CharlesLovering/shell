AUTHOR: Charlie Lovering
EMAIL: cjlovering@wpi.edu

FUNCTIONS:

int main(int args, char* argv[]);
   
   If the main is called without arguements, it will enter a shell simulator.

int execArgs(char* argv[], bool background, struct job* listofjobs[]);
    
    This function is forks and manages the execution of inputted commands. This function will
    wait for the execution of the commands if it is not a background process.

int shell(struct job *listofjobs[]);

    This function is called by the execArgs, and consists of calls to prompt and process.

char* prompt();
    
    This function prompts the user, by offering the symbols '>>>' and waits for user input,
    and then returns it to the shell function.

int process(char* result, job *listofjobs[]);

    This function processes input commands, removing the '\n' and extra spaces. It also checks 
    for special commands, such as 'cd', 'exit' and 'jobs.'

char* processString(char* s);

    This removes the leading spaces from the command.

char* readline();

    This gets and parses the line from the user.

void displayData(struct timeval begin, struct timeval end, struct rusage ru);
    
    This displays data and usage statistics. 

int capture(int pid, int index, struct job *listofjobs[], bool background);

    This function waits for an executing process, and then prints out information of that process by calling displayData.    

int incrementIndex();

    Increments the index of the jobs array.

int getIndex();

    Returns the current index.    

void jobs(int index, struct job *listofjobs[]);

    Displays the current running jobs. (Not working.)

int addToList(struct job *new_job, struct job *listofjobs[], int index);

    Adds a job to the array of jobs.

struct job* findFinishedJob(int fpid, struct job* listofjobs[]);

    Finds the finished job.

char * convertCommands(char* commands[]);
     
    Converts the char** commands to a char* so it can be displayed.
