#include <stdio.h>     /* standard I/O functions                         */
#include <stdlib.h>    /* exit                                           */
#include <unistd.h>    /* standard unix functions, like getpid()         */
#include <signal.h>    /* signal name macros, and the signal() prototype */

/* first, define the Ctrl-C counter, initialize it with zero. */
int ctrl_c_count = 0;
int got_response = 0;
#define CTRL_C_THRESHOLD  5 

/* the Ctrl-C signal handler */
void catch_int(int sig_num)
{
  /* increase count, and check if threshold was reached */
  ctrl_c_count++;
  if (ctrl_c_count >= CTRL_C_THRESHOLD) {
    char answer[30];
    alarm(10); //must initialize the alarm() which calls SIGALRM signal after 10 seconds that the user hasn't responded

    /* prompt the user to tell us if to really
     * exit or not */
    printf("\nReally exit? [Y/n]: ");
    fflush(stdout);//fflush flushes out the stream
    fgets(answer, sizeof(answer), stdin);//this gets the user input
    if (answer[0] == 'n' || answer[0] == 'N') {
       //by setting alarm to 0 it ensures that no new SIGALRM is called for and the 10 second one declared above remains
      //essentially making sure that the SIGALRM keeps counting incase the user is not responding 
      alarm(0);
      printf("\nContinuing\n"); //continues if answer is 'n' or 'N'
      fflush(stdout);

      /* 
       * Reset Ctrl-C counter
       */
      ctrl_c_count = 0;
    }
    else {
      printf("\nExiting...\n");
      fflush(stdout);
      exit(0);
    }
  }
}

/* the Ctrl-Z signal handler */
void catch_tstp(int sig_num)
{
  /* print the current Ctrl-C counter */
  printf("\n\nSo far, '%d' Ctrl-C presses were counted\n\n", ctrl_c_count);
  fflush(stdout);
}

/* function must be created in order to catch the 10 second alarm that was initialized in the previous code
 * catch_alarm simply calls the SIGALRM signal if the alarm(10) call is triggered
 */
void catch_alarm(int sig_num){
  printf("\n User taking too long to respnd. Exiting . . .\n");
  exit(0);
}

int main(int argc, char* argv[])
{
  struct sigaction sa_a;
  struct sigaction sa_b;
  struct sigaction sa_c;
  sigset_t mask_set;  /* used to set a signal masking set. */
  /* setup mask_set */
  //initializes and fills the signal test
  sigfillset(&mask_set);
  //deletest the signal from the signal set SIGALRM
  sigdelset(&mask_set, SIGALRM);
  //initialize the sa_mask pointers
  sa_a.sa_mask = mask_set;
  sa_b.sa_mask = mask_set;
  sa_c.sa_mask = mask_set;



  /* set signal handlers */
  //sa_handler is a pointer that points to the catch_int handler
  sa_a.sa_handler = catch_int;
  //sa_handler is a pointer that points to the catch_tsp handler
  sa_c.sa_handler = catch_tstp;
  //sa_handler is a pointer that points to the catch_alarm handler
  sa_b.sa_handler = catch_alarm;
  

  /*set action for the signal actions*/
  //changes the signal action SIGINT 
  sigaction(SIGINT, &sa_a, NULL);
  //changes the ginal action SIGTSTP
  sigaction(SIGTSTP, &sa_c, NULL);
  //changes the signal action SIGALRM
  sigaction(SIGALRM, &sa_b, NULL);

  //runs while true and waits 
  //without this while loop the program instantly ends 
  while(1){
    pause();
  }
  return 0;
}

