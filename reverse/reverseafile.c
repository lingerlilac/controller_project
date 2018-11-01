/*
The script below should open a file, read and reverse every line, then overwrite the source file.
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// reverses the order of characters in every line of SRC file

void validate_cmdline_args(int argc, char **argv) {
  if(argc != 2) {
    printf("Usage: %s SRC\n", argv[0]);
    exit(1);
  }
}

void reverse_string(char *str) {
  char *end = strchr(str, (int)'\0') - 1;
  char tmp;
  while(str < end) {
    tmp = *str;
    *str++ = *end;
    *end-- = tmp;
  }
}

void process_line(char *output_content, char *line) {
  reverse_string(line);
  strcat(output_content, line);
  strcat(output_content, "\n");
}

int main(int argc, char **argv) {
  validate_cmdline_args(argc, argv);

  char* file_content = NULL;
  char* output_content = NULL;
  char *file_content_free_pointer = NULL;

  int file_descriptor;
  if((file_descriptor = open(argv[1], O_RDONLY)) == -1)
    goto errno_error;

  struct stat file_stat;
  if(fstat(file_descriptor, &file_stat) == -1)
    goto errno_error;

  if((file_content = malloc((size_t)file_stat.st_size+1)) == NULL)
    goto errno_error;
  file_content_free_pointer = file_content;

  if(read(file_descriptor, file_content, (size_t)file_stat.st_size) == -1)
    goto errno_error;
  file_content[(int)file_stat.st_size] = '\0';

  if(close(file_descriptor) == -1)
    goto errno_error; 

  if((output_content = malloc((size_t)file_stat.st_size+2)) == NULL)
    goto errno_error;
  output_content[0] = '\0';

  char* delim = "\n";
  char* line = NULL;

  while((line = strsep(&file_content, delim)))
    process_line(output_content, line);

  if((file_descriptor = open(argv[1], O_WRONLY | O_TRUNC)) == -1)
    goto errno_error;

  if(write(file_descriptor, output_content, (size_t)file_stat.st_size) == -1)
    goto errno_error;
  if(close(file_descriptor) == -1)
    goto errno_error; 

  free(file_content_free_pointer);
  free(output_content);

  return 0;
errno_error:
  perror(NULL);
  free(file_content_free_pointer);
  free(output_content);
  return 1;
}