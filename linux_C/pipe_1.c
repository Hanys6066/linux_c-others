#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define DATA_LEN  10			

#define READ_BUFF_SIZE	5	/*number of long double items which can be store in read buffer*/
long double data_to_send[DATA_LEN] = {1.0L, 1.5L, 2.1L, 2.2L, 3.1L, 1.2L, 8.8L, 9.7L, 1.5L, 10.0L};
long double result_array[DATA_LEN]; 
size_t buf_size = sizeof(long double);

void child_main_func(int w_pipe)
{
	/*write dat in pipe one by one*/
	int i;
	size_t num_to_write = sizeof(long double);
	for(i = 0; i < DATA_LEN; i++){
		if(num_to_write != write(w_pipe, (void *)(&data_to_send[i]), num_to_write)){
			printf("child: error during write()\n");
			_exit(EXIT_FAILURE);
		}
	}
	if(-1 == close(w_pipe)){
		printf("child: error during closing write end\n");
		_exit(EXIT_FAILURE);
	}
}

void parent_main_func(int r_pipe)
{
	int i;
	long double read_buff[READ_BUFF_SIZE];
	static unsigned int res_index = 0;
	ssize_t num_read;
	for(;;){
		num_read = read(r_pipe, (void *)(&read_buff), buf_size * READ_BUFF_SIZE);
		if(num_read == -1){
			printf("parent: error during read()");
			exit(EXIT_FAILURE);
		}
		if(num_read == 0){/*end of communication*/
			printf("EOF\n");
			break;
		}
		printf("%d number was readed\n", (int)(num_read / sizeof(long double)));
		/*copy data to result array*/
		for(i = 0; i < (int)(num_read / sizeof(long double)); i++){
			printf("read_buff[%u] = %llf\n", res_index + i ,read_buff[i]);
			result_array[res_index + i] = read_buff[i];
		}
		res_index += (unsigned int)(num_read/sizeof(long double));
	}
	close(r_pipe);	
}

int main(int argc, char **argv)
{
	int pfd[2];
	int i;

	if(-1 == pipe(pfd)){
		printf("error in pipe()\n");
		return(-1);
	}
	switch(fork()){
		case -1:
			printf("error during fork\n");
			return(-1);

		case 0: /*child process-> write into pipe*/
			if(-1 == close(pfd[0])){	/*close unused end of pipe*/
				printf("child: error during closing read end of pipe\n");
				_exit(EXIT_FAILURE);			
			}
			child_main_func(pfd[1]);
			_exit(EXIT_SUCCESS);
	
		default:/*parent->read data and print them*/
			/*read data by NUM_READ_NUMBERS*/		
			if(-1 == close(pfd[1])){	/*close unused end of pipe*/
				printf("parent: error during closing write end of pipe\n");
				exit(EXIT_FAILURE);
			}
			parent_main_func(pfd[0]);
			wait(NULL);	/*wait for child*/
			/*print result array*/
			for(i = 0; i < DATA_LEN; i++){
				printf("array[%d]: %llf\n", i, result_array[i]);
			}
			exit(EXIT_SUCCESS);
	}		
}
