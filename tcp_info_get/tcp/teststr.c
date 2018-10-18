#include <stdio.h>
#include <string.h>

int strlinger ( char *buf, char *sub)
{
    if(!*sub)
        return -1;
    char *bp, *sp;
    int ind = 0;
    while (*buf)
    {
        bp = buf;
        sp = sub;
        do
        {
            if(!*sp)
                return ind;
        } while (*bp++ == *sp++);
        buf++;
        ind++;
    }
    return -1;
}

main()
{
    char *s="Golden:GlobalView";
    char *l=":";
    int p = 0;
     
    p=strlinger(s,l);
    if(p >= 0)
        printf("%s %d\n", s + p, p);
    else
        printf("NotFound!\n");
    return 0;
}