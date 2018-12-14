main: src/main.cpp
	mpicxx -O3 -g -rdynamic -o main src/main.cpp

clean:
	rm main