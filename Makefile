main: player.o playerUtil.o
	g++ -Wall -o main *.o main.cpp -lmpv

test: player.o playerUtil.o
	g++ -Wall -o test *.o tests/testUtil.cpp -lmpv

player.o: player.cpp player.h
	g++ -c player.cpp 

playerUtil.o: playerUtil.cpp playerUtil.h
	g++ -c playerUtil.cpp

clean:
	rm *.o
	rm main
	rm test
