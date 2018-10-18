objects =  createtree.o

helloworld: $(objects)
	gcc -o createtree $(objects)
createtree.o: createtree.c 
	gcc -c createtree.c 
clean:
	rm -f *.o createtree