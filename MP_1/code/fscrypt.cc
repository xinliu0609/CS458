#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openssl/blowfish.h"

const int BLOCKSIZE = 8;

void *fs_encrypt(void *plaintext, int bufsize, char *keystr, int *resultlen)
{
	BF_KEY key;
	BF_set_key(&key, strlen(keystr)+1, (const unsigned char *)keystr);

	// padding if the length of the plaintext is not a multiple of 8

	if (bufsize % BLOCKSIZE != 0)
	{
		// construct the padding byte array
		int padding_length = BLOCKSIZE * (bufsize / BLOCKSIZE + 1) - bufsize;
		unsigned char *padding_string = (unsigned char *)malloc(sizeof(char) * padding_length);

		for(int i = 0; i < padding_length; i++)
		{
			padding_string[i] = padding_length;
		}

		// construct a new byte array, then padding
		unsigned char *new_plaintext = (unsigned char *)malloc(sizeof(char) * (bufsize+padding_length));
		memcpy(new_plaintext, plaintext, bufsize);
		memcpy(new_plaintext+bufsize, padding_string, padding_length);

		plaintext = new_plaintext;
		new_plaintext = NULL;

		bufsize = bufsize+padding_length;
	}

	unsigned char *plain = (unsigned char *)plaintext;
	unsigned char *result = (unsigned char *)malloc(bufsize * sizeof(char));
	unsigned char *previous_vector = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));
	
	for(int i = 0; i < BLOCKSIZE; i++)
	{
		previous_vector[i] = 0;	// initial vector is all NULL character
	}

	BF_cbc_encrypt(plain, result, bufsize, &key, previous_vector, BF_ENCRYPT);
	
	*resultlen = bufsize;

	plain = NULL;
	free(previous_vector);

	return (void *)result;
}

//////////////////////////////////////////////////////////////////////////////////////

void *fs_decrypt(void *ciphertext, int bufsize, char *keystr, int *resultlen)
{
	BF_KEY key;
	BF_set_key(&key, strlen(keystr)+1, (const unsigned char *)keystr);

	unsigned char *cipher = (unsigned char *)ciphertext;
	unsigned char *result = (unsigned char *)malloc(bufsize * sizeof(char));

	unsigned char *previous_vector = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));
	
	for(int i = 0; i < BLOCKSIZE; i++)
	{
		previous_vector[i] = 0;	// initial vector is all NULL character
	}

	BF_cbc_encrypt(cipher, result, bufsize, &key, previous_vector, BF_DECRYPT);

	// Now, truncate the padding bytes
	int padding_length = (int) result[bufsize-1];

	if(padding_length != 0)
	{
		bufsize = bufsize - padding_length;
		
		unsigned char *true_result = (unsigned char *)malloc(bufsize * sizeof(char));
		memcpy(true_result, result, bufsize);
		result = true_result;
		true_result = NULL;
	}

	*resultlen = bufsize;
	cipher = NULL;

	free(previous_vector);

	return (void *)result;
}
