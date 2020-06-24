#include "my_file.h"


/*_________________________Global Function__________________________*/
char *readfd_line(int fd, Boolean change_pos);
char *base_name(const char *path);
char *RelPath_To_AbsPath(const char *rel_path);
long long fileSize(const int fd);
struct RDWR_buff RDWR_buffSize(const size_t true_len, const size_t optimal_buff);
struct read_data read_file_data(const int fd, const size_t buff_size, const char *mode);
void clean_read_data(struct read_data *data);

/*________________________Function Definitions_______________________*/

char *readfd_line(int fd, Boolean change_pos)
{
    /*read one line from actual position, if change_pos == TRUE,
    shift file offset at current location(at the end of reade line),
    retunrned string has to be deallocated!!!!*/
    char *line = (char *)NULL;
    char c[1] = {[0] = 0};
    size_t r_stat = 0, pos = 0, mult = 1;
    off_t origin = 0;

    origin = lseek(fd, 0, SEEK_CUR);
    REALLOC(line, READ_LINE_BUFF_SIZE, 1, "readfd_line", TRUE);
    while(1){
        r_stat = read(fd, c, 1);    /*read file by one character->very slow*/
        if(r_stat == -1){
            errMsg("readfd_line(): some error occutr during reading\n");
            return((char *)NULL);
        }else{
            if(c[0] == '\n'){
                break;
            }else{
                if(pos >= (mult * READ_LINE_BUFF_SIZE - 1)){/*last character is for '\0'*/
                    /*increase the buffer*/
                    REALLOC(line, mult * READ_LINE_BUFF_SIZE, 1, "readfd_line", TRUE);
                    mult++;
                }
                line[pos] = c[0];
            }
        }
        pos++;
    }
    line[pos] = '\0';
    if(change_pos == FALSE){
        if(origin != lseek(fd, -origin, SEEK_CUR)){
            errMsg("readfd_line(): cannot set file offset back");
        }
    }
    return(line);
}


char *RelPath_To_AbsPath(const char *rel_path)
{
  /*--------------------create absolute path---------------------*/
  /*param-> rel_path: array consisting relative path*/
  /*Path is return as dynamic allocated array and has to be unallocated by user*/
  size_t pos, rel_pos = 0;
  char *path = (char *)NULL;
  char curr_path[PATH_MAX_LEN_REL_PATH] = {0};
  size_t len, i;

  if(NULL == getcwd(curr_path, sizeof(curr_path))){
    errMsg("getcwd() error");
    return ((char *)NULL);
  }

  pos = strlen(curr_path);/*position of last character*/
  len = strlen(rel_path);/*len of rel path*/
  len += pos + 1;/*len of whole path, +1 because of terminating null byte*/

  /*allocated array for absolute path*/
  REALLOC(path, len, sizeof(char), "RelPath_To_AbsPath", TRUE);
  /*copy actual path to absolute path*/
  for(i = 0; i < len; i++){
      path[i] = curr_path[i];
  }
  /*add rel path to path of project*/
  /*if first character is '.' skip*/
  if(rel_path[rel_pos] == '.'){
    rel_pos++;/*skip character*/
    if(rel_path[rel_pos] == '.'){
        rel_pos++; /*skip '.' characters*/
        if(rel_path[rel_pos] == '.'){
            free(path);
            return((char *)NULL);
        }
    }
  }
  do{
    path[pos] = rel_path[rel_pos];
    rel_pos++;
    pos++;
}while(rel_path[rel_pos]);
  return path;
}


char *base_name(const char *path)
{
  /**this function return file name from path, !!return array is created by malloc
  and must be later deallocated**/
  char *file_name = (char *)NULL;
  printf("base_name: %s\n", path);
  char tmp[PATH_MAX_LEN_FILE_NAME];
  unsigned int count = 0, pos = 0;

  while(1){
    if((path[pos] == 92) || (path[pos] == 47) || (path[pos] == '\0')){/*\ and / and null*/
      if(path[pos] == '\0'){
        unsigned int i;
        REALLOC(file_name, (count + 1), sizeof(char), "base_name(): file_name", TRUE);
        for(i = 0;i < count; i++){
            file_name[i] = tmp[i];
        }
        file_name[count] = '\0';
        break;
    }else{
          tmp[count] = '\0';
          pos++;/*skip \ or / character*/
          /*if end of string break and return*/
          count = 0;
          continue;
      }
    }
    if(pos >= PATH_MAX_LEN_FILE_NAME - 1){
      errno = ERANGE;
      printf("function base name is too long");
      break;

    }
    tmp[count] = path[pos];
    pos++;
    count++;
  }
  return file_name;
}

/*obatin size of file*/
long long fileSize(const int fd)
{

  off_t cur, start, end;

  if((cur = lseek(fd, 0, SEEK_CUR)) == -1){
    errMsg("fileSize failure: lseek()");
    return((long long)-1);
  }

  if((start = lseek(fd, 0, SEEK_SET)) == -1){
    errMsg("fileSize failure: lseek()");
    return((long long)-1);
  }

  if((end = lseek(fd, 0, SEEK_END)) == -1){
    errMsg("fileSize failure: lseek()");
    return(-(long long)-1);
  }

  if(lseek(fd, cur, SEEK_SET) == -1){
    errMsg("fileSize failure: lseek()");
    return((long long)-1);
  }

  return((long long)(end - start));
}

struct RDWR_buff RDWR_buffSize(const size_t true_len, const size_t optimal_buff)
{
    size_t t_buff_size;
    size_t numIter = 0;
    struct RDWR_buff buff_data;

    if(true_len > optimal_buff){
        if(optimal_buff > SSIZE_MAX){
            t_buff_size = SSIZE_MAX;
            numIter = (size_t)ceil(true_len / SSIZE_MAX);
        }else{
            t_buff_size = optimal_buff;
            numIter = (size_t)ceil(true_len/ optimal_buff);
        }
    }else{
        t_buff_size = true_len;
        numIter = (size_t)1;
    }

    if((numIter * t_buff_size) > true_len){
        buff_data.last_buff = true_len - ((numIter- 1) * t_buff_size);
    }

    buff_data.num_iter = numIter;
    buff_data.buff_size = t_buff_size;

    return buff_data;
}



struct read_data read_file_data(int fd, const size_t buff_size, const char *mode)
{
    struct read_data rd;
    size_t num, pos, i;
    long long tmp;
    char buff_tmp[buff_size];
    char *start = (char *)NULL;

    rd.data = (char *)NULL;
    rd.file = (FILE *)NULL;

    if(-1 ==(tmp = fileSize(fd))){
        rd.isvalid = FALSE;
        errMsg("read_file_data(): error in file_size()");
        return(rd);
    }

    rd.size = (size_t)tmp + 1;/*\0 character*/

    if(NULL == (rd.file = fdopen(fd, mode))){
        rd.isvalid = FALSE;
        errMsg("read_file_data(): error in fdopen()");
        return(rd);
    }

    REALLOC(rd.data, rd.size, 1, "read_Scaler_from_file", FALSE);
    start = rd.data;

    while(1){
        num = fread(buff_tmp, sizeof(char), buff_size, rd.file);
        memcpy(rd.data, buff_tmp, num);
        pos += num;
        /*shift data->data*/
        rd.data += num;
        if(num != (size_t)buff_size){
            if(feof(rd.file) != 0){
                /*end of file*/
                *(rd.data++) = '\0';
                rd.data = start;
                rd.isvalid = TRUE;
                //Data_Proc_ladim_print_data(data->data);
                break;
            }else if(ferror(rd.file) !=0){
                /*some error occure*/
                errMsg("line_reading(): error\n");
                rd.isvalid = FALSE;
            }
        }else{
            /*below statment has to be here. because case when size of file is exactly
            multiple of buff_size*/
            rd.isvalid = TRUE;
        }
    }
    return rd;
}

void clean_read_data(struct read_data *data)
{
    free(data->data);
    data->data = (char *)NULL;
    data->file = (FILE *)NULL;
}
