/*
 * smm.cpp - test the SimpleSharedMapMemory class by creating a "/MySharedMem" memory
 * map, and then reporting this current process' pid in a continuous loop.
 * 
 * Testing: See the 'make test' target of the Makefile.
 */
#include <string>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <vector>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#endif


#include "SimpleSharedMapMemory.h"

// This struct will be shared between processes.  It contains a simple list of
// processes that share the "/MySharedMem" memory map.
struct SharedData {
    int pid_count;
    int pids[100];
};

// A simple example - creates the "/MySharedMem" memory map if it doesn't exist
// or re-uses it, if it already exists, populates an entry in the SharedData for
// the current process (my_pid), and reports on the data structure in an 
// infinite loop for test purposes.  Run two copies of this process to see
// the results.
int main() {
    try {
        const size_t REQUESTED_SIZE = sizeof(SharedData);
        std::cerr << "SharedData size: " << REQUESTED_SIZE << std::endl;
        if (REQUESTED_SIZE <= 0) {
            throw std::runtime_error("Shared memory size is invalid");
        }
        
        bool is_creator = true;
        SimpleSharedMapMemory* shm = NULL;
        try {
            shm = new SimpleSharedMapMemory("/MySharedMem", REQUESTED_SIZE, true);
        }
        catch (const std::exception& e) {
            std::cerr << "Creator failed: " << e.what() << std::endl;
            is_creator = false;
            try {
                shm = new SimpleSharedMapMemory("/MySharedMem", REQUESTED_SIZE, false);
            }
            catch (const std::exception& e2) {
                std::cerr << "Joiner also failed: " << e2.what() << std::endl;
                throw;
            }
        }

        SharedData* data = static_cast<SharedData*>(shm->get());
        
        if (is_creator) {
            data->pid_count = 0;
            memset(data->pids, 0, sizeof(data->pids));
        }

#ifdef _WIN32
        int my_pid = GetCurrentProcessId();
#else
        int my_pid = getpid();
#endif

        bool pid_added = false;
        for (int i = 0; i < 100; i++) {
            if (data->pids[i] == 0 && !pid_added) {
                data->pids[i] = my_pid;
                data->pid_count++;
                pid_added = true;
            }
        }

        while (true) {
            std::cout << "Process " << my_pid << " sees PIDs: ";
            std::vector<int> active_pids;
            for (int i = 0; i < 100; i++) {
                if (data->pids[i] != 0) {
                    active_pids.push_back(data->pids[i]);
                    std::cout << data->pids[i] << " ";
                }
            }
            std::cout << "(Total: " << data->pid_count << ")" << std::endl;

            for (size_t i = 0; i < active_pids.size(); i++) {
#ifdef _WIN32
                HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, active_pids[i]);
                if (h == NULL) {
                    for (int j = 0; j < 100; j++) {
                        if (data->pids[j] == active_pids[i]) {
                            data->pids[j] = 0;
                            data->pid_count--;
                            break;
                        }
                    }
                }
                else {
                    CloseHandle(h);
                }
#else
                if (kill(active_pids[i], 0) == -1 && errno == ESRCH) {
                    for (int j = 0; j < 100; j++) {
                        if (data->pids[j] == active_pids[i]) {
                            data->pids[j] = 0;
                            data->pid_count--;
                            break;
                        }
                    }
                }
#endif
            }

#ifdef _WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
        }

        delete shm;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
