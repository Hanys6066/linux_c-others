#ifndef MY_FILE_H
#define MY_FILE_H

#include <unistd.h>
#include <fcntl.h>
#include "../my_error/my_error.h"
#include <string.h>

#define PATH_MAX_LEN_REL_PATH       1024
#define PATH_MAX_LEN_FILE_NAME      128
#define FILE_RDWR_BUFF_SIZE         2048
#define READ_LINE_BUFF_SIZE         128

struct read_data{
    FILE            *file;
    size_t           size;
    char            *data;
    Boolean       isvalid;
};


/*structure is used by function RDWR_buffSize,
structure store information about number of
iteration(num_iter) which has to be perform to read or write
data by buffer of size(buff_size)*/
struct RDWR_buff{
    size_t num_iter;
    size_t buff_size;
    size_t last_buff;       /*size of bufer during last iteration can be different*/
};

extern char *readfd_line(int fd, Boolean change_pos);
/*read line at current file offset, if change_pos is
FALSE than file offset reamin unchanged(as it was before call of this function),
returned string is dynamickly allocated and has to be deallocated*/

extern long long fileSize(const int fd);
/*function return size of data in file->using file offset->function lseek()*/

extern char *RelPath_To_AbsPath(const char *rel_path);
/*--------------------create absolute path---------------------*/
/*param-> rel_path: array consisting relative path*/
/*Path is return as dynamic allocated array and has to be unallocated by user*/

extern char *base_name(const char *path);
/*return file_name from path, array has to be deallocated, when it is not needed*/

extern struct RDWR_buff RDWR_buffSize(const size_t true_len, const size_t optimal_buff);
/*return size of buffer, which is equal to optimal_buff if this size is smaller than SSIZE_MAX
constant for actual machine, and if true_len is smaller than optimal_buff*/

extern struct read_data read_file_data(const int fd, const size_t reg_buff_size, const char *mode);
/*function which return read_data structure, which contains string obtained from
file data, file size and file pointer and finally isvalid flag which is set on FALSE if some
error occur, this function needs file descriptor fd and size of buffer, which speciefid
size of buffer, which is used by fread function, this function doesnt set buffer mode->
if we want change or set buffer mode we must set this before*/
extern void clean_read_data(struct read_data *data);
/*deallocate all dynamiclly allocated items within read_data structure*/
#endif
