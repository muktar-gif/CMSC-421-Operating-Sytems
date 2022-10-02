CC = gcc

mailboxTest: mailboxTest.c mailboxSyscalls.h
	$(CC) mailboxTest.c -o mailboxTest

run:
	./mailboxTest

sudo:
	sudo ./mailboxTest

sudoValgrind:
	sudo valgrind ./mailboxTest