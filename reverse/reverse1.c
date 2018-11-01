/*
This program will print the contents of given file name in reverse order. Just like tac command in Linux. tac command displays contents of the file in reverse order. Same as tac command in this program you have to supply file name.
The syntax of executing this program
./program-name file-name

C program to print contents in reverse order of a file.
linux tac command implementation.
*/

//Write a program to print reverse content of file
#include <stdio.h>
#include <string.h>
 
int main(int argc, char *argv[])
{
    FILE *fp1;
     
    int cnt = 0;
    int i   = 0;
     
    if( argc < 2 )
    {
        printf("Insufficient Arguments!!!\n");
        printf("Please use \"program-name file-name\" format.\n");
        return -1;
    }
     
    fp1 = fopen(argv[1],"r");
    if( fp1 == NULL )
    {
        printf("\n%s File can not be opened : \n",argv[1]);
        return -1;
    }
     
    //moves the file pointer to the end.
    fseek(fp1,0,SEEK_END);
    //get the position of file pointer.
    cnt = ftell(fp1);
     
    while( i < cnt )
    {
        i++;
        fseek(fp1,-i,SEEK_END);
        printf("%c",fgetc(fp1));
    }
    printf("\n");
    fclose(fp1);
     
    return 0;
}
