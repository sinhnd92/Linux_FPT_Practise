Type make clean followed by make all to build the library
Copy the libiofunc.a into /usr/lib
Copy the iolib.h into /usr/include
compile: gcc test_app.c –liofunc –o test_app  
Execute your program as usual using ./main_app