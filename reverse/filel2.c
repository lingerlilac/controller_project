/* 
 * C Program to Reverse the Contents of a File and Print it
 */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <linux/types.h>
#include <string.h>
long count_characters(FILE *);

static void get_systime_linger(__be32 *t, __be32 *v)
{
  struct timeval tv;
  memset(&tv, 0, sizeof(struct timeval));
  gettimeofday(&tv, NULL);
  *t = tv.tv_sec;
  *v = tv.tv_usec;
}

void main(int argc, char * argv[])
{
    int i;
    long cnt;
    char ch, ch1;
    FILE *fp1, *fp2;
    __be32 sec1, sec2;
    __be32 usec1, usec2;
    __be32 duration = 0;
    get_systime_linger(&sec1, &usec1);
    if (fp1 = fopen(argv[1], "r"))    
    {
        printf("The FILE has been opened...\n");
        fp2 = fopen(argv[2], "w");
        cnt = count_characters(fp1); // to count the total number of characters inside the source file
        fseek(fp1, -1L, 2);     // makes the pointer fp1 to point at the last character of the file
        printf("Number of characters to be copied %d\n", ftell(fp1));
 
        while (cnt)
        {
            ch = fgetc(fp1);
            fputc(ch, fp2);
            fseek(fp1, -2L, 1);     // shifts the pointer to the previous character
            cnt--;
        }
        printf("\n**File copied successfully in reverse order**\n");
    }
    else
    {
        perror("Error occured\n");
    }
    fclose(fp1);
    fclose(fp2);
    get_systime_linger(&sec2, &usec2);
    duration = (sec2 - sec1) * 1000000 + (usec2 - usec1);
    printf("duration is %d\n", (int)duration);
}
// count the total number of characters in the file that *f points to
long count_characters(FILE *f) 
{
    fseek(f, -1L, 2);
    long last_pos = ftell(f); // returns the position of the last element of the file
    last_pos++;
    return last_pos;
}
/*
C Program to Reverse the Contents of a File and Print it
This C Program reverses the contents of a file and print it.
Here is source code of the C Program to reverse the contents of a file and print it. The C program is successfully compiled and run on a Linux system. The program output is also shown below.
$ gcc file12.c
$ cat test2
The function STRERROR returns a pointer to an ERROR MSG STRING whose contents are implementation defined.
THE STRING is not MODIFIABLE and maybe overwritten by a SUBSEQUENT Call to the STRERROR function.
$ a.out test2 test_new
The FILE has been opened..
Number of characters to be copied 203
 
**File copied successfully in reverse order**
$ cat test_new
 
.noitcnuf RORRERTS eht ot llaC TNEUQESBUS a yb nettirwrevo ebyam dna ELBAIFIDOM ton si GNIRTS EHT
.denifed noitatnemelpmi era stnetnoc esohw GNIRTS GSM RORRE na ot retniop a snruter RORRERTS noitcnuf ehT
$ ./a.out test_new test_new_2
The FILE has been opened..
Number of characters to be copied 203
 
**File copied successfully in reverse order**
$ cat test_new_2
The function STRERROR returns a pointer to an ERROR MSG STRING whose contents are implementation defined.
THE STRING is not MODIFIABLE and maybe overwritten by a SUBSEQUENT Call to the STRERROR function.
$ cmp test test_new_2
*/