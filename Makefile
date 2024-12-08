CC := g++
CFLAGS := -pthread -lsqlite3 -Iinc
SRC := $(shell find src -type f -name '*.cpp')
TARGET := Sensor_Gateway

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CFLAGS)

clean:
	rm Sensor_Gateway gateway.log sensor_data.db
