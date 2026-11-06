all:
	gcc main.c postoffice.c -o postoffice -lpthread -Wall
clean:
	rm -f postoffice