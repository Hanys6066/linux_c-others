#define __GNU_SOURCE

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#define BUF_SIZE 200
static volatile sig_atomic_t canJump = 0;
static volatile sig_atomic_t end = 0;
#ifdef USE_SIGSETJMP
static sigjmp_buf senv;
#else
static jmp_buf env;
#endif


static void handler(int sig)
{
  if(sig == SIGINT){
    write(STDIN_FILENO,"sigint\n",7);
    end = 1;
  }

  if(sig == SIGALRM){
     if(canJump){
     /*this condition is here because it is necessarily ensure that
	setjmp was done before handler is invoked */
	return;
     }

     write(STDIN_FILENO, "ALARM\n",6);

     #ifdef USE_SIGSETJMP
        siglongjmp(senv,1);
     #else
        longjmp(env, 1);
     #endif

  }
}


int main(int argc, char **argv){
  int first_pass = 1;
  struct itimerval itv, old;
  static struct timeval start;
  struct timeval curr;
  struct sigaction sa;
  char buf[BUF_SIZE];
  int num_read = 0;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  sa.sa_handler = handler;
  itv.it_value.tv_sec = 5;
  itv.it_value.tv_usec = 0;
  itv.it_interval.tv_sec = 5;
  itv.it_interval.tv_usec = 0;

  if(sigaction(SIGALRM, &sa, NULL) == -1){
    exit(EXIT_FAILURE);
  }

  if(sigaction(SIGINT, &sa, NULL )== -1){
    exit(EXIT_FAILURE);
  }

  if(setitimer(ITIMER_REAL, &itv, NULL)){
    exit(EXIT_FAILURE);
  }

  for(;end == 0;){
    if(first_pass == 1){
      #ifdef USE_SIGSETJMP
        if(sigsetjmp(senv,1) == 0)
      #else
        if(setjmp(env) == 0)
      #endif
      canJump = 1;
      printf("set timer\n");
      first_pass = 0;
    }else{
      num_read = read(STDIN_FILENO, buf, BUF_SIZE);

     if(num_read == -1){
       if(errno = EINTR)
	 printf("Timer expires\n");
       else{
	 printf("read was unsucessful\n");
       }
     }else{

       if(setitimer(ITIMER_REAL, &itv, &old)){
	 exit(EXIT_FAILURE);
       }

       printf("you write: %s left: %ds  %dus\n", buf, old.it_value.tv_sec,
		old.it_value.tv_usec);
     }
    }
  }
  return 0;

}
