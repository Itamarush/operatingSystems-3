CFLAGS=-Wall -O2
LDFLAGS=-ldl -lcrypto
TARGET = stnc

#SOURCES = stnc.c utils.c tcpUtils.c udpUtils.c pipeUtils.c mmapUtils.c udsUtils.c
SOURCES = stnc.c
OBJECTS = $(SOURCES:.c=.o)
RPATH=-Wl,-rpath=./

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
