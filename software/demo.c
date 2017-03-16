/*  
*   Byte-oriented AES-256 implementation.
*   All lookup tables replaced with 'on the fly' calculations. 
*
*   Copyright (c) 2007 Ilya O. Levin, http://www.literatecode.com
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose with or without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies.
*
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "aes256.h"

void print_results(char * s, FILE * fhandle, uint8_t buf[], int size) {

	int i = 0;

	fprintf(fhandle, "%s", s);

	for (i = 0; i < size; i++) {
		fprintf(fhandle, "%02x", buf[i]);
	}

	fprintf(fhandle,"\n");

	return;
}

#define LINELENGTH 96

int main (int argc, char *argv[]) {

    aes256_context ctx; 

    int line = 1;
    int errors = 0;

    uint8_t key[32];
    uint8_t buf[16], i;

    FILE * input_file;
    FILE * output_file;

    char buffer [LINELENGTH];

    char strHexData[2];

    unsigned int keybyte[32];
    unsigned int databyte[16];
    unsigned int encbyte[16];


    // check for input text file argument -- print help if needed
   if(argc != 2 || strcmp(argv[1], "-h") == 0) {
        fprintf(stderr, "Usage: ./demo <input file>\n\n");
        return -1;
	}

	// attempt to open input text file -- return error if not found
    input_file = fopen(argv[1], "r");

    if (input_file == NULL) {
    	fprintf(stderr, "ERROR: Could not open input file. Check <file location> argument.\n");
    	return -1;
    }

    // attempt to open output text file -- return error if it fails
    output_file = fopen ("output.txt", "w+");

    if (output_file == NULL) {
    	fprintf(stderr, "ERROR: Could not open output file... check location where program is running.\n");
    	return -1;
    }

    fprintf(output_file, "AES Software Results:\n\n");

    // read input line-by-line
    while(fgets(buffer, LINELENGTH, input_file) != NULL) {

    	int j = 0;
    	int k = 0;

    	// read the key data from input file
    	if (buffer[0] == 'k') {

			for (k = 2; k < 66; k=k+2) {

				strHexData[0] = buffer[k];
				strHexData[1] = buffer[k+1];

				sscanf(strHexData, "%x", &keybyte[j]);
				j++;

			}
		}

		// read the plaintext data from input file
		else if (buffer[0] == 't') {

			for (k = 2; k < 34; k=k+2) {

				strHexData[0] = buffer[k];
				strHexData[1] = buffer[k+1];

				sscanf(strHexData, "%x", &databyte[j]);
				j++;
			}
		}

		// read the encrypted data from input file
		else if (buffer[0] == 'e') {

			for (k = 2; k < 34; k=k+2) {

				strHexData[0] = buffer[k];
				strHexData[1] = buffer[k+1];

				sscanf(strHexData, "%x", &encbyte[j]);
				j++;
			}
		}

		// run SW encryption/decryption and compare
		// against the HW results (from input file)
		else if (buffer[0] == 'r') {

		    // create test key
		    for (i = 0; i < sizeof(buf); i++) {
		    	buf[i] = databyte[i];
		    }

		    // create test input data
		    for (i = 0; i < sizeof(key); i++) {
		    	key[i] = keybyte[i];
		    }

		    // encrypt data and print results
			print_results("key = ", output_file, key, sizeof(key));
			print_results("text_in = ", output_file, buf, sizeof(buf));

		    aes256_init(&ctx, key);
		    aes256_encrypt_ecb(&ctx, buf);

		    print_results("cipher_text = ", output_file, buf, sizeof(buf));

		    // check HW encryption against SW encryption
			for (i = 0; i < sizeof(buf); i++) {

				if (buf[i] != encbyte[i]) {
					errors += 1;
					fprintf(stdout, "buf[i] = %d, encbyte[i] = %d\n", buf[i], encbyte[i]);
				}
			}

			// decrypt results and print
		    aes256_init(&ctx, key);
		    aes256_decrypt_ecb(&ctx, buf);
		    print_results("text_out = ", output_file, buf, sizeof(buf));

		    fprintf(output_file,"\n");
		    aes256_done(&ctx);
		}

		// skip newline characters in trace file
		else if (buffer[0] == '\n') {}
		
		else {fprintf(stderr, "\nERROR: Failed on line %d - check the input text file.\n", line);}

		// increment the line for debugging and keeping track
		line++;
	}

		// software program over -- print results
		fprintf(output_file, "There were %4d errors found between software & hardware.\n\n", errors);
		fprintf(output_file, "END OF FILE");

		// close both input and output files
		fclose(input_file);
		fclose(output_file);

		// let CMD line know we are finished
		fprintf(stdout, "Finished running AES software validation. Check 'output.txt' for results.\n\n");

	    return 0;
}