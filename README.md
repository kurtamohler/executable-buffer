# executable-buffer

This repo provides an ExecutableBuffer C++ class which can be used to
generate, modify, and execute machine code at runtime. In other words,
this is a library to enable you to create self-modifying code.

## Supported Environments

Currently this library only works on Linux machines.

## Example Applications

### Build

```
$ make all
```

### NOP example

This example simply fills and executes the buffer with NOP instructions.
