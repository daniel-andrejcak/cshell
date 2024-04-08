CC = gcc
CFLAGS = -Wall
SRCS = main.c nodes/server.c nodes/client.c nodes/server/parse_commands.c 
OBJS = $(SRCS:.c=.o)
TARGET = danielshell


all: objects $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(addprefix objects/,$(notdir $(OBJS)))

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o objects/$(notdir $@)

objects:
	mkdir -p objects

clean:
	rm -f $(OBJS) $(TARGET)