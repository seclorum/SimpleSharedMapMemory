/* 
 * SimpleSharedMapMemory - a simple way to share memory-mapped memory between two processes.
 */
#pragma once
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

class SimpleSharedMapMemory {
public:
    SimpleSharedMapMemory(const std::string& name, size_t size, bool create = true)
        : m_name(name), m_size(size), m_ptr(NULL), m_handle(
#ifdef _WIN32
            INVALID_HANDLE_VALUE
#else
            -1
#endif
        ), m_is_creator(create) {
        if (size == 0) {
            throw std::invalid_argument("Shared memory size must be greater than 0");
        }
        std::cerr << "Attempting to " << (create ? "create" : "open") 
                  << " shared memory '" << name << "' with size " << size << std::endl;
        open(create);
    }

    ~SimpleSharedMapMemory() {
        close();
    }

#if __cplusplus >= 201103L
    SimpleSharedMapMemory(const SimpleSharedMapMemory&) = delete;
    SimpleSharedMapMemory& operator=(const SimpleSharedMapMemory&) = delete;
    SimpleSharedMapMemory(SimpleSharedMapMemory&& other) noexcept
        : m_name(std::move(other.m_name)), m_size(other.m_size),
          m_ptr(other.m_ptr), m_handle(other.m_handle), m_is_creator(other.m_is_creator) {
        other.m_ptr = NULL;
        other.m_handle = 
#ifdef _WIN32
            INVALID_HANDLE_VALUE;
#else
            -1;
#endif
        other.m_is_creator = false;
    }
    SimpleSharedMapMemory& operator=(SimpleSharedMapMemory&& other) noexcept {
        if (this != &other) {
            close();
            m_name = std::move(other.m_name);
            m_size = other.m_size;
            m_ptr = other.m_ptr;
            m_handle = other.m_handle;
            m_is_creator = other.m_is_creator;
            other.m_ptr = NULL;
            other.m_handle = 
#ifdef _WIN32
                INVALID_HANDLE_VALUE;
#else
                -1;
#endif
            other.m_is_creator = false;
        }
        return *this;
    }
#endif

    void* get() const { return m_ptr; }
    size_t size() const { return m_size; }

private:
    std::string m_name;
    size_t m_size;
    void* m_ptr;
#ifdef _WIN32
    HANDLE m_handle;
#else
    int m_handle;
#endif
    bool m_is_creator;

    void open(bool create) {
#ifdef _WIN32
        DWORD creationDisposition = create ? CREATE_ALWAYS : OPEN_EXISTING;
        m_handle = CreateFileMappingA(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            static_cast<DWORD>(m_size),
            m_name.c_str()
        );
        
        if (m_handle == NULL) {
            throw std::runtime_error("Failed to create/open file mapping: " + 
                                   std::to_string(GetLastError()));
        }

        m_ptr = MapViewOfFile(
            m_handle,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            m_size
        );

        if (m_ptr == NULL) {
            CloseHandle(m_handle);
            m_handle = INVALID_HANDLE_VALUE;
            throw std::runtime_error("Failed to map view of file: " + 
                                   std::to_string(GetLastError()));
        }
#else
        int flags = O_RDWR;
        if (create) {
            flags |= O_CREAT | O_EXCL;
            m_handle = shm_open(m_name.c_str(), flags, S_IRUSR | S_IWUSR);
            if (m_handle == -1) {
                if (errno == EEXIST) {
                    throw std::runtime_error("Shared memory already exists");
                }
                throw std::runtime_error("Failed to create shared memory: " + 
                                       std::string(strerror(errno)));
            }

            std::cerr << "Creating with fd " << m_handle << " and size " << m_size << std::endl;
            if (ftruncate(m_handle, m_size) == -1) {
                int saved_errno = errno;
                ::close(m_handle);
                shm_unlink(m_name.c_str());
                m_handle = -1;
                throw std::runtime_error("Failed to set shared memory size: " + 
                                       std::string(strerror(saved_errno)));
            }
            struct stat sb;
            if (fstat(m_handle, &sb) == -1) {
                throw std::runtime_error("Failed to verify size: " + 
                                       std::string(strerror(errno)));
            }
            std::cerr << "Created shared memory with actual size: " << sb.st_size << std::endl;
            m_size = sb.st_size;
        } else {
            m_handle = shm_open(m_name.c_str(), flags, S_IRUSR | S_IWUSR);
            if (m_handle == -1) {
                throw std::runtime_error("Failed to open shared memory: " + 
                                       std::string(strerror(errno)));
            }
            struct stat sb;
            if (fstat(m_handle, &sb) == -1) {
                int saved_errno = errno;
                ::close(m_handle);
                m_handle = -1;
                throw std::runtime_error("Failed to get shared memory size: " + 
                                       std::string(strerror(saved_errno)));
            }
            m_size = sb.st_size;
            std::cerr << "Opened existing shared memory with size: " << m_size << std::endl;
        }

        m_ptr = mmap(NULL, m_size, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    m_handle, 
                    0);

        if (m_ptr == MAP_FAILED) {
            int saved_errno = errno;
            ::close(m_handle);
            if (m_is_creator) shm_unlink(m_name.c_str());
            m_handle = -1;
            throw std::runtime_error("Failed to map shared memory: " + 
                                   std::string(strerror(saved_errno)));
        }
#endif
    }

    void close() {
        if (m_ptr) {
#ifdef _WIN32
            UnmapViewOfFile(m_ptr);
            if (m_handle != INVALID_HANDLE_VALUE) {
                CloseHandle(m_handle);
            }
#else
            if (munmap(m_ptr, m_size) == -1) {
                std::cerr << "Warning: Failed to unmap memory: " << strerror(errno) << std::endl;
            }
            if (m_handle != -1) {
                if (::close(m_handle) == -1) {
                    std::cerr << "Warning: Failed to close handle: " << strerror(errno) << std::endl;
                }
                if (m_is_creator) {
                    if (shm_unlink(m_name.c_str()) == -1) {
                        std::cerr << "Warning: Failed to unlink " << m_name << ": " 
                                << strerror(errno) << std::endl;
                    }
                }
            }
#endif
            m_ptr = NULL;
            m_handle = 
#ifdef _WIN32
                INVALID_HANDLE_VALUE;
#else
                -1;
#endif
        }
    }
};



