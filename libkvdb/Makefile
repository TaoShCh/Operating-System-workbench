NAME := $(shell basename $(PWD))
export TASK := M6
all: $(NAME)-64.so $(NAME)-32.so
CFLAGS += -U_FORTIFY_SOURCE

run32 :
	make
	gcc -m32 -o a.out test.c libkvdb-32.so -lpthread -Wl,-rpath=./
	./a.out

run64 :
	make
	gcc -o a.out test.c libkvdb-64.so -lpthread -Wl,-rpath=./
	./a.out

include ../Makefile
include ../Makefile.lab
