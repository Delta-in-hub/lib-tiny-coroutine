CC=clang
test:test32 test64
	@echo "test32"
	./test32 >> /dev/null
	@echo "test64"
	./test64 >> /dev/null
rtest:clean test
test32:
	$(CC) -m32 -Wall -g -O1 -fPIC test.c co.c -o test32
test64:
	$(CC) -m64 -Wall -g -O1 -fPIC test.c co.c -o test64
clean:
	 rm ./test32 ./test64
