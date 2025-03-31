# SimpleSharedMapMemory
A simple C++ class to implement shared, mapped memory, intended for sharing data between two independent processes.


# Usage
For an example, see smm.cpp.

To test, first build with 'make', and to test, 'make test'.  

To use it, simply include the "SimpleSharedMapMemory.h" header file in your project.

# Example Output

	$ make test
	g++ -std=c++11 smm.cpp -o smm   # -lrt needed on some POSIX systems for shm_open
	timeout 15 ./smm &
	sleep 3
	SharedData size: 404
	Attempting to create shared memory '/MySharedMem' with size 404
	Creator failed: Shared memory already exists
	Attempting to open shared memory '/MySharedMem' with size 404
	Opened existing shared memory with size: 16384
	Process 30342 sees PIDs: 29641 30342 (Total: 2)
	Process 30342 sees PIDs: 30342 (Total: 1)
	Process 30342 sees PIDs: 30342 (Total: 1)
	timeout 5 ./smm &
	SharedData size: 404
	Attempting to create shared memory '/MySharedMem' with size 404
	Creator failed: Shared memory already exists
	Attempting to open shared memory '/MySharedMem' with size 404
	Opened existing shared memory with size: 16384
	Process 30350 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30350 30342 (Total: 2)
	Process 30350 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30350 30342 (Total: 2)
	Process 30350 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30350 30342 (Total: 2)
	Process 30350 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30350 30342 (Total: 2)
	Process 30350 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30350 30342 (Total: 2)
	Process 30342 sees PIDs: 30342 (Total: 1)
	Process 30342 sees PIDs: 30342 (Total: 1)
	Process 30342 sees PIDs: 30342 (Total: 1)
	Process 30342 sees PIDs: 30342 (Total: 1)
	Process 30342 sees PIDs: 30342 (Total: 1)
	Process 30342 sees PIDs: 30342 (Total: 1)
