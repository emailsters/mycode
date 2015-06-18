MBIN=main
$(MBIN):main.o
	g++ -o $(MBIN) main.o -g
main.o:
	g++ -c main.cpp
clean:
	rm -rf $(MBIN) main.o
