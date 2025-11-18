compile: main.cpp
	g++ -std=c++17 -O3 -march=native -pthread main.cpp -o main.out

compile-sanitized: main.cpp
	g++ -std=c++17 -O3 -march=native -pthread -fsanitize=thread main.cpp -o main.out

run: main.out
	./main.out

clean:
	@rm -rf *.out *.o
