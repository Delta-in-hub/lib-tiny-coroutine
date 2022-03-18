CC=gcc
test:test32 test64
	@echo "test32"
	./test32
	@echo "test64"
	./test64
rtest:clean test
test32:
	$(CC) -m32 -Wall -g -O1 -fPIC test.c co.c -o test32
test64:
	$(CC) -m64 -Wall -g -O1 -fPIC test.c co.c -o test64
clean:
	 rm ./test32 ./test64
