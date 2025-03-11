CC = g++
CFLAGS = -std=c++17 -Wall -Werror
LDFLAGS = -lpthread
SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)

build: $(OBJS)
	$(CC) $(CFLAGS) -o inverted-index $^ $(LDFLAGS)

clean:
	rm inverted-index $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<
