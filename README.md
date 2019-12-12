# Project 4: Network File System

## About This Project
This projects is responsible for implementing the FUSE file system in a manner that establishes a linux implementation through TCP connection.

### How does our server side work?
Our server side takes in arguments that generate its main functionality; ''' port number''' and .

### How does our client side work?
Realloc adjusts the size of our previously allocated block if it is within the range of the old block size. If a the block usage is not suitable for resizing into the requested new size, we will call malloc to create a new memory block. If a zero is passed in then we free the block and update the linked list.

### Included Files
There are several files included. These are:
   - <b>Makefile</b>: For adjusting File specifics
   - <b>README.md</b>: it is a me, readme
   - <b>allocator.c</b>: this is a file that holds our memory managment functions
   - <b>allocator.h</b>: this file holds th metadata of the allocator file (our header) 
   - <b>debug.h</b>: holds helper functions to debug our source code


## Testing

To check test cases, a check on a seleced mount file could be implemented where both the server and client should be running and commands should be executed within the mounted file.

