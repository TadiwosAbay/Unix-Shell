STUDENT_ID := $(shell cat STUDENT_ID)
SUBMIT_DIR := .submit
SUBMIT_FILES:= *.c *.h Makefile readme EthicsOath.pdf
SUBMIT := $(STUDENT_ID)_assign5.tar.gz

CC = gcc209
CCFLAGS = -g  -D_GNU_SOURCE

TARGET = ish

# FIX THIS FILE

all: $(TARGET)
ish: ish.o dfa.o syntactic.o dynarray.o
	$(CC) $(CCFLAGS) ish.o dfa.o syntactic.o dynarray.o -o ish
ish.o: ish.c syntactic.c dfa.c dfa.h dynarray.h
	$(CC) $(CCFLAGS) -c ish.c 
syntactic.o: syntactic.c dfa.c dfa.h dynarray.h
	$(CC) $(CCFLAGS) -c syntactic.c
dfa.o: dfa.c dfa.h dynarray.h
	$(CC) $(CCFLAGS) -c dfa.c
dynarray.o: dynarray.c
	$(CC) $(CCFLAGS) -c dynarray.c


submit:
	mkdir -p $(SUBMIT_DIR)
	cp $(SUBMIT_FILES) $(SUBMIT_DIR)
	cd $(SUBMIT_DIR) && tar -czf ../$(SUBMIT) *
	rm -rf $(SUBMIT_DIR)

clean:
	rm -rf $(TARGET) *.o

.PHONY: all clean submit
