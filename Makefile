
# The first target is the default if you just say "make".
build: appendtofile

appendtofile: AppendToFile.o
	gcc -g -o $@ AppendToFile.o

#I did all my tests manually because the program takes user input. 
#It seemed more tedious and time consuming to actually write the tests out
#here and check each one rather than one at a time manually. Also could
#not figure out how to efficiently pipe multiple data entries into this program.
test:	
	@echo "Read makefile comments for test"
	

clean:
	rm -f appendtofile *.core *.o *.jg3538
