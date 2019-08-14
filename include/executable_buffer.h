/**

ExecutableBuffer class

ExecutableBuffer provides a buffer that can be toggled
between executable mode and writable mode. This provides
the ablity to generate and then execute machine code at
runtime.


Supported environments:
    * Linux


Implementation details:

    mmap() is used to allocate the buffer to ensure that
    memory regions used by other parts of the application
    are not affected when toggling between memory protection
    modes.

    mprotect() is used to toggle memory protection modes.

**/


#ifndef __EXECUTABLE_BUFFER_H__
#define __EXECUTABLE_BUFFER_H__

#include <cstddef>
using namespace std;

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
    //   length - the desired length (bytes) of the buffer
    //
    ExecutableBuffer(size_t length);

    ////////////////////////////////////////////////////////
    //
    // Destructor
    //
    // Details:
    //   munmap is called to destroy the buffer.
    //   
    ~ExecutableBuffer();

    ////////////////////////////////////////////////////////
    //
    // Get the length (bytes) of the allocated buffer.
    //
    // Returns: Length (bytes) of allocated buffer.
    //
    size_t getAllocLength();

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
    // Get a pointer to the buffer
    //
    // Returns: Pointer to the beginning of the buffer
    //
    void* const begin();

private:
    int setProtection(int prot);
    void flushCache();

    size_t alloc_length;
    void* buffer;
};


#endif // #ifndef __EXECUTABLE_BUFFER_H__
