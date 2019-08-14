#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include "executable_buffer.h"

using namespace std;

typedef ExecutableBuffer eb;

eb::ExecutableBuffer(size_t length) :
    executable(false)
{
    if (length == 0) {
        length++;
    }

    // If length is not a multiple of the page size, we need to
    // increase it
    size_t page_size = getpagesize();

    if ((length % page_size) != 0) {
        this->alloc_length = page_size * ((length / page_size) + 1);

    } else {
        this->alloc_length = length;
    }

    // Create read/write-able buffer
    this->buffer = mmap(
        NULL,
        this->alloc_length,
        PROT_READ | PROT_WRITE,
        MAP_ANON | MAP_PRIVATE,
        -1,
        0
    );

    if (this->buffer == (caddr_t) -1) {
        cerr << "ERROR: could not allocate buffer with mmap" << endl;
        exit(1);
    }
}

eb::~ExecutableBuffer() {
    munmap(this->buffer, this->alloc_length);
}

size_t eb::getAllocLength() {
    return this->alloc_length;
}

int eb::setProtection(int prot) {
    return mprotect(
        this->buffer,
        this->alloc_length,
        prot
    );
}

void eb::setExecutable() {
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

void eb::setWritable() {
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

unsigned long eb::execute() {
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

void eb::flushCache() {
    __builtin___clear_cache(
        (char*) this->buffer,
        (char*) this->buffer+this->alloc_length-1
    );
}

void* const eb::begin() {
    return this->buffer;
}
