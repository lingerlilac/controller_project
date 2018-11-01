/*
Beginning C, Third Edition
Reading strings from a file in reverse order
 By Ivor Horton
 ISBN: 1-59059-253-0
 Published: Apr 2004
 Publisher: apress

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LENGTH 1000
#define BUFFER_SIZE 50

/* Global variables */
FILE *pInFile = NULL;                   /* File pointer to input file     */
FILE *pOutFile = NULL;                  /* File pointer  to output file   */
char *buffer = NULL;
size_t buffer_size = BUFFER_SIZE;


void main(int argc, char * argv[])
{
  size_t str_length = 0;
  int str_count = 0;
  fpos_t *positions = NULL;
  int i = 0;

  buffer = (char*)malloc(buffer_size);            /* Create initial buffer */

  if((pInFile = fopen(argv[1], "r")) == NULL)  /* Open the input file   */
  {
    printf("Error opening %s for reading. Program terminated.", argv[1]);
    abort();
  }

  /* Find out how many strings there are */
  for(;;)
  {
    fread(&str_length, sizeof(size_t), 1, pInFile);  /* Read the string length */
    if(feof(pInFile))                                /* If it is end of file   */
      break;                                         /* We are finished        */

    /* Check buffer is large enough and increase if necessary */
    if(str_length>buffer_size)
    {
      buffer_size = str_length+1;
      free(buffer);
      buffer = (char*)malloc(buffer_size);
    }
    fread(buffer, str_length, 1, pInFile);   /* Read the string */
    ++str_count;
  }
  printf("\nThere are %d strings in the input file.", str_count);

  /* Now get the position for the beginning of each record in the file */
  /* The buffer is now large enough to hold the longest string         */
  rewind(pInFile);
  positions = (fpos_t*)malloc(str_count*sizeof(fpos_t));  /* Array to store the positions */
  for(i = 0 ; i<str_count ; i++)
  {
    fgetpos(pInFile, positions+i);                    /* Get the positions      */
    fread(&str_length, sizeof(size_t), 1, pInFile);   /* Read the string length */
    fread(buffer, str_length, 1, pInFile);            /* Read the string        */
 }

  /* Open the output file */
  if((pOutFile = fopen(argv[2], "w")) == NULL)
  {
    printf("Error opening %s for reading. Program terminated.", argv[2]);
    abort();
  }

  /* Read the records in reverse order from the input file and write to the new file */
  for(i = 0 ; i<str_count ; i++)
  {
    fsetpos(pInFile, positions+str_count-i-1);            /* Set the file position  */
    fread(&str_length, sizeof(size_t), 1, pInFile);       /* Read the string length */
    fwrite(&str_length, sizeof(size_t), 1, pOutFile);     /* Write to new file      */
    fread(buffer, str_length, 1, pInFile);                /* Read the string        */
    fwrite(buffer, str_length, 1, pOutFile);              /* Write to new file      */
  }

  fclose(pInFile);                                        /* Close input file  */
  fclose(pOutFile);                                       /* Close output file */
  printf("\nNew file write complete\n");

  /* List contents of output file */
  if((pOutFile = fopen(argv[2], "r")) == NULL)        /* Open the new file to read it */
  {
    printf("Error opening %s for reading. Program terminated.", argv[2]);
    abort();
  }
  printf("\nThe strings in the new file are:");
  for(i = 0 ; i<str_count ; i++)
  {
    fread(&str_length, sizeof(size_t), 1, pOutFile);
    fread(buffer, str_length, 1, pOutFile);
    buffer[str_length] = '\0';
    printf("\n%s", buffer);
  }
  printf("\n");
  fclose(pOutFile);                                    /* Close file */

  /* Free the memory we allocated */
  if(buffer != NULL)
    free(buffer);
  if(positions != NULL)
    free(positions);
 }
