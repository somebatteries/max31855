readtemp:
	gcc -o $@ max31855.c

install:
	cp readtemp ../
