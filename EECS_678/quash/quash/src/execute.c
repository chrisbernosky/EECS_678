/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"
#include "deque.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#include "quash.h"
#include <stdlib.h> //getenv lib




#define READEND 0
#define WRITEND 1
#define SIZE 1024
#define TAKEOVER 1

IMPLEMENT_DEQUE_STRUCT(Pids, pid_t);
IMPLEMENT_DEQUE(Pids, pid_t);


struct Job
{
  int job_id;
  Command cmd;
  const char* command;
  Pids pids;
  int noProcesses;
  pid_t* pid;
}Job;

IMPLEMENT_DEQUE_STRUCT(Jobs, struct Job);
IMPLEMENT_DEQUE(Jobs, struct Job);

/***************************************************************************
 * Interface Functions
 ***************************************************************************/
 #define IMPLEMENT_ME()                                                  \
   fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)





 void destroy_pipe(int* pipe)
 {
   close(pipe[READEND]);
   close(pipe[WRITEND]);

   free(pipe);

 }

 void _wait(pid_t pid)
 {
   int status;
   waitpid(pid, &status, 0);
 }

 void execute_job(struct Job myJob)
 {
  int job_id = myJob.job_id;
  pid_t pid = peek_front_Pids(&myJob.pids);
  char* cmd = myJob.command;
  print_job(job_id, pid, cmd);
 }


Jobs jobs;
struct Job current_job;
// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  char* wd = get_current_dir_name();
  return wd;
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple


  return getenv(env_var);
}


void write_env(const char* env_var, const char* val)
{
  setenv( env_var, val, TAKEOVER);
}


// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  //IMPLEMENT_ME();

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);



}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;
  execvp(exec,args);
  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  int i = 0;

  while(NULL != str[i])
  {
    printf("%s", str[i]);
    i++;
  }

  printf("\n");

  // Flush the buffer before returning
  //free(str);
  fflush(stdout);
}//END run_echo


void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;
  setenv(env_var, val, TAKEOVER);

}//END run_export


//http://pubs.opengroup.org/onlinepubs/009695399/functions/chdir.html
/*
1) chdir() to change the current directory
2) Update some environment variables OLD_PWD and PWD
3) free blocks
*/
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;
  char* previous;
  char* current;

  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  previous = getcwd(NULL, SIZE);

  chdir(dir);

  current = getcwd(NULL, SIZE);

  setenv("PWD", current, TAKEOVER );
  setenv("OLD_PWD", previous, TAKEOVER);

  free(previous);
  free(current);
}//END run_cd

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {

  int signal = cmd.sig;
  int job_id = cmd.job;

  int job_length = length_Jobs(&current_job);

  for(int i = 0; i < job_length; i++)
  {
    struct Job job;

    if(job_id == job.job_id)
    {
      Pids job_queue;

      int queue_length;

      job_queue = job.pids;
      queue_length = length_Pids(&job_queue);

       for( int j = 0; j < queue_length; j++)
      {
        int pid = pop_front_Pids(&job_queue);
        kill(pid,signal);
        push_back_Pids(&job_queue,pid);
      }
      push_back_Jobs(&current_job,job);

    }
    else
    {
        push_back_Jobs(&current_job, job);
    }
  }

  // // TODO: Remove warning silencers
  // pid_t pid = getppid();
  // job_id = pid;

  // kill(job_id, signal);
}


// Prints the current working directory to stdout
void run_pwd() {
  bool should_free;

  char* str = get_current_directory(&should_free);

  printf("%s\n", str );


  free(str);

  //flush the buffer
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {

  if(is_empty_Jobs(&current_job))
  {
    return;
  }

  int job_length = length_Jobs(&current_job);

  for(int i = 0; i < job_length; i++)
  {
    struct Job job = pop_front_Jobs(&current_job);

    print_job(job.job_id,peek_front_Pids(&job.pid), job.command);

    push_back_Jobs(&current_job, job);
  }
  // // TODO: Print background jobs
  // apply_Jobs(&jobs, execute_job);

  // // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Child Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);
  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:

  default:
    fprintf(stderr, "Parent Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder) {
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND;

  static int environment_pipes[2][2];
  static int prevous_pipe = -1;
  static int next_pipe = 0;

  if (p_out) {
    /* This is the only condition under which a new pipe creation is required.
       You should be able to understand why this is the case */
    pipe (environment_pipes[next_pipe]);
}


pid_t pid = fork ();

if (0 == pid) {
  if(r_in)
    {
      FILE *inFile = fopen(holder.redirect_in, "r" );
      dup2(fileno(inFile),STDIN_FILENO);
    }
    if(r_out)
    {
      if(r_app)
      {
        FILE *f_out = fopen(holder.redirect_out, "a" );
        dup2(fileno(f_out),WRITEND);
      }
      else
      {
        FILE *f_out = fopen(holder.redirect_out, "w" );
        dup2(fileno(f_out),WRITEND);
      }
    }
    /* Check if this process needs to receive from previous process */
    if (p_in) {
        dup2 (environment_pipes[prevous_pipe][READEND], STDIN_FILENO);


        /* We are never going to write to the previous pipe so we can safely close it */
        close (environment_pipes[prevous_pipe][WRITEND]);
    }

    if (p_out) {
        dup2 (environment_pipes[next_pipe][WRITEND], STDOUT_FILENO);

        /* We are never going to read from our own pipe so we can safely close it */
        close (environment_pipes[next_pipe][READEND]);
    }

    /* Execute what-ever needs to be run in the child process */
    child_run_command (holder.cmd);

    /* Adios child process */
    exit (EXIT_SUCCESS);
} else {
    /* Close the hanging pipes in parent */
    int wstatus;
      waitpid(-1, &wstatus, 0);
    if (p_out) {
        close (environment_pipes[next_pipe][WRITEND]);
    }
    if(p_in)
      {
        close(environment_pipes[prevous_pipe][0]);
      }
      if(get_command_type(holder.cmd) == EXPORT ||get_command_type(holder.cmd) == CD || get_command_type(holder.cmd) == KILL)
         {
           parent_run_command(holder.cmd);
         }
    /* Update the pipe trackers for next iteration */
    next_pipe = (next_pipe + 1) % 2;
    prevous_pipe = (prevous_pipe + 1) % 2;
}

/* This function can safely kick  the bucket now */
return;
}


void run_script(CommandHolder* holders) {
  if (holders == NULL)
    return;
  check_jobs_bg_status();
  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }
  CommandType type;
  struct Job thisJob;
  Pids pids;
  int Pipes[sizeof(holders)][2];
  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i]) ) != EOC; ++i){
    create_process(holders[i]);
  }



  if (!(holders[0].flags & BACKGROUND)) {

    // apply_Pids(&thisJob.pids, _wait);
    // destroy_Pids(&thisJob.pids);

    while(!is_empty_Pids(&pids)) {
      int status;
        if (
          waitpid(peek_back_Pids(&current_job.pids),
            &status, 0) != -1)
            {
              pop_back_Pids(&current_job.pids);
            }
          }
    destroy_Pids(&pids);

  }


  else {
    // A background job.
    // TODO: Push the new job to the job queue
    // //IMPLEMENT_ME();
    //
    // // TODO: Once jobs are implemented, uncomment and fill the following line
    // // print_job_bg_start(job_id, pid, cmd);
    // push_back_Jobs(&jobs, thisJob);
    
    
    // int job_id = thisJob.job_id;
    // pid_t pid = peek_front_Pids(&thisJob.pids);
    // char* cmd = thisJob.command;
    
    // print_job_bg_start(job_id, pid, cmd);
  }
}//END run_script