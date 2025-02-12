main: player.o
	g++ -Wall -o main *.o main.cpp -lmpv

player.o: player.cpp player.h
	g++ -c player.cpp 

clean:
	rm *.o
	rm main
