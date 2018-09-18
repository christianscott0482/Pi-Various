TARGET = morsecode
CFLAGS = -g -Wall
OBJS = morse.o morse.h
CC = gcc
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)
clean:
	rm -f $(TARGET) $(OBJS)
