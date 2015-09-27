#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openssl/blowfish.h"

// implement the encrypt / decrypt function using the openSSL library

/*

1.	void BF_set_key(BF_KEY *key, int len, const unsigned char *data)

2.	void BF_ecb_encrypt(const unsigned char *in, unsigned char *out, BF_KEY *key, int enc)

*/


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
		bufsize = bufsize+padding_length;
	}

	unsigned char *plain = (unsigned char *)plaintext;

	// use blowfish to encrypt
	unsigned char *result = (unsigned char *)malloc(bufsize * sizeof(char));
	unsigned char *result_offset = result;
	unsigned char *plain_offset = plain;

	unsigned char *previous_vector = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));
	
	for(int i = 0; i < BLOCKSIZE; i++)
	{
		previous_vector[i] = 0;	// initial vector is all NULL character
	}

	// pure ecb, each block of plain text is encrypted using Blowfish
	/*
	for(int i = 0; i < bufsize/BLOCKSIZE; i++)
	{
		unsigned char *out = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));
		BF_ecb_encrypt(plain, out, &key, BF_ENCRYPT);

		memcpy(result_offset, out, BLOCKSIZE);
		result_offset += BLOCKSIZE;
		plain += BLOCKSIZE;

		free(out);
	}
	*/
	
	// implement cbc by myself

	for(int i = 0; i < bufsize/BLOCKSIZE; i++)
	{
		unsigned char *out = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));

		for(int i = 0; i < BLOCKSIZE; i++)
		{
			previous_vector[i] ^= plain_offset[i];
		}

		BF_ecb_encrypt(previous_vector, out, &key, BF_ENCRYPT);

		memcpy(result_offset, out, BLOCKSIZE);
		result_offset += BLOCKSIZE;
		plain_offset += BLOCKSIZE;

		for(int i = 0; i < BLOCKSIZE; i++)
		{
			previous_vector[i] = out[i];
		}

		free(out);
	}
	
	*resultlen = bufsize;

	plain = NULL;
	result_offset = NULL;
	plain_offset = NULL;

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
	unsigned char *result_offset = result;
	unsigned char *cipher_offset = cipher;

	unsigned char *previous_vector = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));
	
	for(int i = 0; i < BLOCKSIZE; i++)
	{
		previous_vector[i] = 0;	// initial vector is all NULL character
	}
	
	// homemade cbc
	
	for(int i = 0; i < bufsize/BLOCKSIZE; i++)
	{
		unsigned char *out = (unsigned char *)malloc(BLOCKSIZE * sizeof(char));

		BF_ecb_encrypt(cipher_offset, out, &key, BF_DECRYPT);

		for(int i = 0; i < BLOCKSIZE; i++)
		{
			out[i] ^= previous_vector[i];
		}

		memcpy(result_offset, out, BLOCKSIZE);

		for(int i = 0; i < BLOCKSIZE; i++)
		{
			previous_vector[i] = cipher_offset[i];
		}

		result_offset += BLOCKSIZE;
		cipher_offset += BLOCKSIZE;

		free(out);
	}

	// Now, truncate the padding bytes
	int padding_length = (int) result[bufsize-1];

	if(padding_length != 0)
	{
		bufsize = bufsize - padding_length;
		
		unsigned char *true_result = (unsigned char *)malloc(bufsize * sizeof(char));
		memcpy(true_result, result, bufsize);
		result = true_result;
	}

	*resultlen = bufsize;

	cipher = NULL;
	result_offset = NULL;
	cipher_offset = NULL;

	free(previous_vector);

	return (void *)result;
}