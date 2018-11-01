.PHONY:all
all:filel2  reverse1  reverse3  reverseafile

filel2:filel2.c
	gcc -o $@ $^
reverse1:reverse1.c
	gcc -o $@ $^
reverse3:reverse3.c
	gcc -o $@ $^
reverseafile:reverseafile.c
	gcc -o $@ $^
.PHONY:clean
clean:
	rm -f filel2  reverse1  reverse3  reverseafile *.o