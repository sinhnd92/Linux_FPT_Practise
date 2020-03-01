all :
	# Compiling with Position Independent Code
	gcc -c -Wall -Werror -fpic sharesumlib.c
	# Creating a shared library from an object file. Note that share library must format with "lib-----.so"
	gcc -shared -o libsharesumlib.so sharesumlib.o

install:
	# Linking with a shared library
	# First, copy file .so to usr/lib
	$(shell sudo cp libsharesumlib.so /usr/lib)
	# Change mode
	$(shell sudo chmod 0755 /usr/lib/libsharesumlib.so)
	# create a link to shared library and update the cache so it is available for immediate use
	$(shell sudo ldconfig)
	# Compile source code
	gcc -Wall -o test test.c -lsharesumlib
clean:
	rm *.so *.o test
