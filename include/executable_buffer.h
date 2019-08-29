//////////////////////////////////////////////////////////////////////
//
// ExecutableBuffer class
//
// ExecutableBuffer provides a buffer that can be toggled between
// executable mode and writable mode. This provides the ablity to
// generate and then execute machine code at runtime.
//
//
// Supported environments:
//   * Linux
//
//
// Implementation details:
//
//   mmap() is used to allocate the buffer to ensure that memory
//   regions used by other parts of the application are not
//   affected when toggling between memory protection modes.
//
//   mprotect() is used to toggle memory protection modes.
//
//////////////////////////////////////////////////////////////////////


#ifndef __EXECUTABLE_BUFFER_H__
#define __EXECUTABLE_BUFFER_H__

#include <cstddef>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <vector>
using namespace std;

template <class T>
class ExecutableBuffer{
public:
    ////////////////////////////////////////////////////////
    //
    // Constructor
    //
    // Details:
    //   The buffer is allocated and set to read/write mode.
    //   The length of the buffer allocated may be slightly
    //   larger than requested, since mmap requires lengths
    //   to be multiples of the system's page size.
    //
    // Arguments:
    //   length - the desired length (number of elements of
    //            type T) for the buffer
    //
    ExecutableBuffer(size_t length);

    ////////////////////////////////////////////////////////
    //
    // Destructor
    //
    // Details:
    //   munmap is called to destroy the buffer.
    //   
    ~ExecutableBuffer(){
        munmap(this->buffer, this->alloc_length_bytes);
    }

    ////////////////////////////////////////////////////////
    //
    // Get the length (bytes) of the allocated buffer.
    //
    // Returns: Length (bytes) of allocated buffer.
    //
    size_t getAllocLength() {
        return this->alloc_length_bytes;
    }

    ////////////////////////////////////////////////////////
    //
    // Set the buffer to executable mode.
    //
    void setExecutable();

    ////////////////////////////////////////////////////////
    //
    // Set the buffer to writable mode.
    //
    void setWritable();


    ////////////////////////////////////////////////////////
    //
    // Execute the code in the buffer
    //
    // Returns: Time (nanoseconds) taken to run the code
    //
    unsigned long execute();

    ////////////////////////////////////////////////////////
    //
    // Get a pointer to the buffer
    //
    // Returns: Pointer to the beginning of the buffer
    //
    T* const begin(){
        return (T* const) this->buffer;
    }


    ////////////////////////////////////////////////////////
    //
    // Element getter
    //
    // Arguments:
    //   ind - the index of the element
    //
    // Returns: Value of element at specified index
    //
    T operator[](int ind) const {
        return this->buffer[ind];
    }

    ////////////////////////////////////////////////////////
    //
    // Element setter
    //
    // Arguments:
    //   ind - the index of the element
    //
    // Returns: Reference to element at specified index
    //
    T& operator[](int ind) {
        return this->buffer[ind];
    }

    ////////////////////////////////////////////////////////
    //
    // 
    void copyVector(vector<T> v, int ind);

private:
    int setProtection(int prot);
    void flushCache();

    size_t alloc_length_bytes;
    T* buffer;
    bool executable;
};

template <class T>
ExecutableBuffer<T>::ExecutableBuffer(size_t length) :
    executable(false)
{
    if (length == 0) {
        length++;
    }

    size_t length_bytes = length * sizeof(T);

    // If length is not a multiple of the page size, we need to
    // increase it
    size_t page_size = getpagesize();

    if ((length_bytes % page_size) != 0) {
        this->alloc_length_bytes = page_size * ((length_bytes / page_size) + 1);

    } else {
        this->alloc_length_bytes = length_bytes;
    }

    // Create read/write-able buffer
    void* buffer = mmap(
        NULL,
        this->alloc_length_bytes,
        PROT_READ | PROT_WRITE,
        MAP_ANON | MAP_PRIVATE,
        -1,
        0
    );

    if (buffer == (caddr_t) -1) {
        cerr << "ERROR: could not allocate buffer with mmap" << endl;
        exit(1);
    }

    this->buffer = (T*) buffer;
}

template <class T>
int ExecutableBuffer<T>::setProtection(int prot) {
    return mprotect(
        this->buffer,
        this->alloc_length_bytes,
        prot
    );
}

template <class T>
void ExecutableBuffer<T>::setExecutable() {
    if (!this->executable) {
        int ret = this->setProtection(PROT_READ | PROT_EXEC);

        if (ret == -1) {
            cerr << "ERROR: could not set buffer to be executable" << endl;
            exit(1);
        }
        
        this->flushCache();

        this->executable = true;
    }
}

template <class T>
void ExecutableBuffer<T>::setWritable() {
    if (this->executable) {
        int ret = this->setProtection(PROT_READ | PROT_WRITE);

        if (ret == -1) {
            cerr << "ERROR: could not set buffer to be executable" << endl;
            exit(1);
        }

        this->flushCache();

        this->executable = false;
    }
}

template <class T>
unsigned long ExecutableBuffer<T>::execute() {
    timespec start_timespec, end_timespec;

    this->setExecutable();

    // Create a function pointer to the beginning of the buffer
    typedef void bufferFuncPtr();
    bufferFuncPtr* execute_buffer = (bufferFuncPtr*) this->buffer;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_timespec);

    // Branch to the beginning of the buffer using the pointer we created
    execute_buffer();

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_timespec);

    unsigned long start_ns, end_ns;

    start_ns = (1000000000 * start_timespec.tv_sec) + start_timespec.tv_nsec;
    end_ns = (1000000000 * end_timespec.tv_sec) + end_timespec.tv_nsec;

    return (end_ns - start_ns);
}

template <class T>
void ExecutableBuffer<T>::flushCache() {
    __builtin___clear_cache(
        (char*) this->buffer,
        (char*) this->buffer+this->alloc_length_bytes-1
    );
}


template <class T>
void ExecutableBuffer<T>::copyVector(vector<T> v, int ind) {
    for (int i = 0; i < v.size(); i++) {
        this->buffer[ind+i] = v[i];
    }
}

#endif // #ifndef __EXECUTABLE_BUFFER_H__
