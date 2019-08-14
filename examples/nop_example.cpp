#include <iostream>
#include "executable_buffer.h"

using namespace std;

int main() {
    const size_t buffer_size = 100000000;
    ExecutableBuffer eb(buffer_size);

    unsigned char* buffer = (unsigned char*) eb.begin();
    unsigned char NOP = 0x90;
    unsigned char RET = 0xc3;
    for (size_t i = 0; i < buffer_size-1; i++) {
        buffer[i] = NOP;
    }

    buffer[buffer_size-1] = RET;

    unsigned long time_ns = eb.execute();
    cout << "starting" << endl;

    cout << "took " << time_ns << " ns to execute "
         << buffer_size << " bytes of NOPs" << endl;

}