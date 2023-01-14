all: crack_checksum

crack_checksum: crack_checksum.c
	$(CC) -O2 -Wall -W crack_checksum.c -o crack-checksum

clean:
	rm -f crack-checksum
