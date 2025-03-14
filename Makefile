CC = gcc

CFLAGS = -Wall -g

SRCS = main.c cmd_parser.c scheduling.c

OBJS = $(SRCS:.c=.o)

TARGET = aubatch

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean