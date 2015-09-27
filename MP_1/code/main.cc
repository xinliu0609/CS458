#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fscrypt.h"

int main()
{
	char s[] = "hello world, and hello Kevin";
	unsigned char *outbuf, *recvbuf;
	char pass[] = "top secret";
	int len = 0;
	int recvlen = 0;

	//printf("the length of plaintext is %zu, and the buf_size is %zu\n", strlen(s), (strlen(s)+1));

	outbuf = (unsigned char *) fs_encrypt( (void *) s, strlen(s)+1, pass, &len );	// s is plain text
																			// strlen(s)+1 is the buffer size, length of plain text
																			// pass is the key
																			// len will be used to store the how many valid bytes
	printf("%s %d\n", "length after encryption =", len);

	int i = 0;
	printf("ciphertext = ");

	for(i = 0; i < len; i++)
	{
		printf("%02x ", outbuf[i]);		// print the Hex presentation of the characters in ASCII
	}
	printf("\n");

	recvbuf = (unsigned char *) fs_decrypt( (void *) outbuf, len, pass, &recvlen);

	assert(memcmp (s, recvbuf, recvlen) == 0);			// memcmp: compare two blocks of memeory
														// compare s and recvbuf, recvlen is the number of bytres to compare
														// return < 0 if first has lower value
														// return > 0 if first has higher value
														// return = 0 if they are equal

	assert(recvlen == (strlen(s) + 1));					// number of valid bytes should be strlen(s)+1

	printf("plaintext = %s\n", recvbuf);
	
	free(outbuf);
	free(recvbuf);
}
