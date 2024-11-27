CC := g++
CFLAGS := -pthread -I.

all:
	$(CC) -o Sensor_Gateway main.cpp Log_Process.cpp $(CFLAGS)

clean:
	rm Sensor_Gateway gateway.log
