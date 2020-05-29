default:
	gcc main.c src/*.c -o main.out -lcommons -lpthread
	./main.out
	rm main.out

test:
	gcc tests/*.c src/*.c -o tests.out -lcspecs -lcommons
	./tests.out
	rm tests.out