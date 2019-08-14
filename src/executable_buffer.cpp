#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
// #include <asm/cachectl.h>
#include "executable_buffer.h"

using namespace std;
typedef ExecutableBuffer eb;

eb::ExecutableBuffer(size_t length) {
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

    // cout << "0x" << hex << (unsigned long) this->buffer << endl;

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
    int ret = this->setProtection(PROT_READ | PROT_EXEC);

    if (ret == -1) {
        cerr << "ERROR: could not set buffer to be executable" << endl;
        exit(1);
    }
    
    this->flushCache();
}

void eb::setWritable() {
    int ret = this->setProtection(PROT_READ | PROT_WRITE);

    if (ret == -1) {
        cerr << "ERROR: could not set buffer to be executable" << endl;
        exit(1);
    }

    this->flushCache();
}

void eb::flushCache() {
    // return cacheflush(
    //     this->buffer,
    //     this->alloc_length,
    //     BCACHE
    // );

    __builtin___clear_cache(
        (char*) this->buffer,
        (char*) this->buffer+this->alloc_length-1
    );
}

void* const eb::begin() {
    return this->buffer;
}
