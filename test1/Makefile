all:docencoder coredump_file
.PHONY:all clean

docencoder:docencoder.o 
	g++ -o docencoder docencoder.o -g
coredump_file:coredump_file.o
	g++ -o coredump_file coredump_file.o -g

docencoder.o:
	g++ -c docencoder.cpp
coredump_file.o:
	g++ -c coredump_file.cpp

clean:
	rm -rf docencoder coredump_file docencoder.o coredump_file.o coredump_notes notes
