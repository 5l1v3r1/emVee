all: main

main: imu.o 
	g++ -c -std=c++11 main.cpp -o main
imu.o: imu.cpp imu.h
	g++ -c -std=c++11 imu.cpp
clean:
	rm *o fps