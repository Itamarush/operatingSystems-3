TARGET = stnc

#SOURCES = stnc.c utils.c tcpUtils.c udpUtils.c pipeUtils.c mmapUtils.c udsUtils.c
SOURCES = stnc.c
OBJECTS = $(SOURCES:.c=.o)
RPATH=-Wl,-rpath=./

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)
