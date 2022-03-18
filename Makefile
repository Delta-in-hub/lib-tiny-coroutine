test:clean all
	@echo ""
	@echo "Start test"
	@echo ""

	./test32gcc_O0.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test64gcc_O0.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test32gcc_Ofast.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test64gcc_Ofast.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test32clang_O0.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test64clang_O0.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test32clang_Ofast.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"
	./test64clang_Ofast.test >> result.log
	@echo $$?
	@echo "This is return value,0 for correct,others for error"


all:gcc_test32_O0 gcc_test64_O0 gcc_test32_Ofast gcc_test64_Ofast clang_test32_O0 clang_test64_O0 clang_test32_Ofast clang_test64_Ofast

clean:
	rm -f ./test*.test ./result.log


gcc_test32_O0:
	gcc -m32 -Wall -g -O0 -fPIC test.c co.c -o test32gcc_O0.test

gcc_test64_O0:
	gcc -m64 -Wall -g -O0 -fPIC test.c co.c -o test64gcc_O0.test

gcc_test32_Ofast:
	gcc -m32 -Wall -g -Ofast -fPIC test.c co.c -o test32gcc_Ofast.test

gcc_test64_Ofast:
	gcc -m64 -Wall -g -Ofast -fPIC test.c co.c -o test64gcc_Ofast.test

clang_test32_O0:
	clang -m32 -Wall -g -O0 -fPIC test.c co.c -o test32clang_O0.test

clang_test64_O0:
	clang -m64 -Wall -g -O0 -fPIC test.c co.c -o test64clang_O0.test

clang_test32_Ofast:
	clang -m32 -Wall -g -Ofast -fPIC test.c co.c -o test32clang_Ofast.test

clang_test64_Ofast:
	clang -m64 -Wall -g -Ofast -fPIC test.c co.c -o test64clang_Ofast.test