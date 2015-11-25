
# The first target is the default if you just say "make".
build: appendtofile

appendtofile: AppendToFile.o
	gcc -g -o $@ AppendToFile.o


test:	build
	(cat testCommands.txt; echo "quit") | ./appendtofile
	
exec:	build
	(cat $(ARG); echo "quit") | ./appendtofile

clean:
	rm -f appendtofile *.core *.o *.jg3538 /tmp/*.jg3538
