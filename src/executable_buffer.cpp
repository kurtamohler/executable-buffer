#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
// #include <asm/cachectl.h>
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
    this->setExecutable();

    #if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
        asm volatile(
            "\t mov %%rax, %[buffer]; \n\t"
            "\t call %%rax; \n\t"
            "\t END_LABEL: \n\t"
            "\t nop; \n\t"
            :
            : [buffer] "r" ((unsigned long) this->buffer)
            : 
        );
    #else
        cerr << "ERROR: Your architecture is not supported. However, "
             << "you can still execute your code if you figure out how"
             << "to write the appropriate assembly code to branch to "
             << "the beginning of your ExecutableBuffer." << endl;
    #endif
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
