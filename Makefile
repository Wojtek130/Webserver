CC=g++
CFLAGS=-std=c++17 -Wall -Wextra
NAME = webserver
RM = rm -f

$(NAME): main.o webserver.o
	$(CC) $(CFLAGS) -g main.o webserver.o -o $(NAME)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o

webserver.o: webserver.cpp webserver.hpp
	$(CC) $(CFLAGS) -c webserver.cpp -o webserver.o

clean:
	rm -f *.o 

distclean: clean
	rm -f $(NAME)