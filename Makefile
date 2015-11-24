
# The first target is the default if you just say "make".
build: appendtofile

appendtofile: AppendToFile.o
	gcc -g -o $@ AppendToFile.o

test:	
	

clean:
	rm -f appendtofile *.core *.o
