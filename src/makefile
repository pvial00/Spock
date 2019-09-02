CC=gcc
CFLAGS=-O3 -std=c99 -w
TARGET=spock
all: $(TARGET)

$(TARGET): spock_cbc_crypt.c
	$(CC) spock_cbc_crypt.c $(CFLAGS) -o $(TARGET)

clean:
	$(RM) $(TARGET)
