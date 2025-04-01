# SimpleSharedMapMemory
A simple cross-platform (Windows/MacOS/Linux) C++ class to implement shared, mapped memory, intended for sharing data between two independent processes.

Note: this is *not* a recommended way to do inter-process communication in the modern era - sandboxing and process isolation are real, and necessary.  However, for internal tooling in a controlled environment, sometimes you need a way for different applications to simply pass large data to each other, or share structures for testing purposes, and in that case a simple drop-in C++ class such as this can be very useful.

Ship this?  No!   Use it to build out a suite of apps/plugins/processes for development purposes .. okay ..


# Usage
For an example, see smm.cpp.  The "SharedData" structure will be placed in the SimpleSharedMapMemory named "MySharedMem" and shared between two copies of the 'smm' process.

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
