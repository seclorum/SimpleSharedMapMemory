smm:	smm.cpp	SimpleSharedMapMemory.h
	g++ -std=c++11 smm.cpp -o smm   # -lrt needed on some POSIX systems for shm_open

test:
	timeout 15 ./smm & 
	sleep 3
	timeout 5 ./smm &

clean:
	rm -rf *.o smm
