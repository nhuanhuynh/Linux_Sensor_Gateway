CC := g++
CFLAGS := -pthread -I.
SRC := $(shell find . -type f -name '*.cpp')
TARGET := Sensor_Gateway

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm Sensor_Gateway gateway.log
