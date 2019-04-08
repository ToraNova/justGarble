/*
 This file is part of JustGarble.

    JustGarble is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    JustGarble is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with JustGarble.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "common.h"
#include "jgutil.h"
#include "justGarble.h"
#include <stdio.h>
#include <ctype.h>

static __m128i cur_seed;

int countToN(int *a, int n) {
	int i;
	for (i = 0; i < n; i++)
		a[i] = i;
	return 0;
}

int dbgBlock(block a) {
	int *A = (int *) &a;
	int i;
	int out = 0;
	for (i = 0; i < 4; i++)
		out = out + (A[i] + 13432) * 23517;
	return out;
}

int compare(const void * a, const void * b) {
	return (*(int*) a - *(int*) b);
}

int median(int *values, int n) {
	int i;
	qsort(values, n, sizeof(int), compare);
	if (n % 2 == 1)
		return values[(n + 1) / 2];
	else
		return (values[n / 2] + values[n / 2 + 1]) / 2;
}

double doubleMean(double *values, int n) {
	int i;
	double total = 0;
	for (i = 0; i < n; i++)
		total += values[i];
	return total / n;
}

void srand_sse(unsigned int seed) {
	cur_seed = _mm_set_epi32(seed, seed + 1, seed, seed + 1);
}

block randomBlock() {

	block cur_seed_split;
	block multiplier;
	block adder;
	block mod_mask;
	block sra_mask;
	block sseresult;

	static const unsigned int mult[4] = { 214013, 17405, 214013, 69069 };
	static const unsigned int gadd[4] = { 2531011, 10395331, 13737667, 1 };
	static const unsigned int mask[4] = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
	static const unsigned int masklo[4] = { 0x00007FFF, 0x00007FFF, 0x00007FFF,
			0x00007FFF };

	adder = _mm_load_si128((block *) gadd);
	multiplier = _mm_load_si128((block *) mult);
	mod_mask = _mm_load_si128((block *) mask);
	sra_mask = _mm_load_si128((block *) masklo);
	cur_seed_split = _mm_shuffle_epi32(cur_seed, _MM_SHUFFLE(2, 3, 0, 1));
	cur_seed = _mm_mul_epu32(cur_seed, multiplier);
	multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
	cur_seed_split = _mm_mul_epu32(cur_seed_split, multiplier);
	cur_seed = _mm_and_si128(cur_seed, mod_mask);
	cur_seed_split = _mm_and_si128(cur_seed_split, mod_mask);
	cur_seed_split = _mm_shuffle_epi32(cur_seed_split, _MM_SHUFFLE(2, 3, 0, 1));
	cur_seed = _mm_or_si128(cur_seed, cur_seed_split);
	cur_seed = _mm_add_epi32(cur_seed, adder);

	return cur_seed;

}

void printint_arr(char *msg, int *t,int start, int size){
	int i;
	printf("%s",msg);
	for(i=start;i<start+size;i++){
		printf("%d ",t[i]);
		if((i+1)%16==0 && i != 0){
			printf("\n");
		}
	}
	printf("\n");
}

void printbin_arr(char *msg,int *t,int size){
	int i;
	printf("%s",msg);
	for(i=0;i<size;i++){
		printf("%d",t[i]);
	}printf("\n");
}

void print128_num(block var){
    u_int16_t *val = (u_int16_t*) &var; //cast the block (128bit value) into 16
    printf("Block value: %5.i %5.i %5.i %5.i %5.i %5.i %5.i %5.i\n",
           val[0], val[1], val[2], val[3], val[4], val[5],
           val[6], val[7]);
}

void printblock_arr(block *il,int size){
	int i;
	printf("Printing block arr:\n");
	for(i=0;i<size;i++){
		print128_num(il[i]);
	}
	printf("\n");
}

void block2bitstream(block *in, int *out, int in_start,int out_start,int no_blocks){
	//convert a block array onto an output int array
	//default blocksize is 128
	int i,j;
	i=in_start;
	for(j=out_start;j<out_start+no_blocks*128;j++){
		out[j] = (((u_int8_t *) in)[ in_start/8 ] & (1<<(in_start % 8))) > 0  ;
		in_start++;
	}
}

void printbitstream(int *target, int totalblocks, int eachblock){
	int i,j;
	printf("Printing bitstream of %d blocks and blocksize %d\n",totalblocks,eachblock);
	printf("<--MSB 	LSB-->\n");
	for(i=0;i<totalblocks;i++){
		for(j=0;j<eachblock;j++){
			printf("%d",target[i*eachblock + j]);
			if((j+1)%4==0 && j != 0){
				printf(" ");
			}
		}
		printf("\n");
	}
	printf("\n");
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

		//ryst from stackoverflow
		//https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

		static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
		                                '4', '5', '6', '7', '8', '9', '+', '/'};
		static int mod_table[] = {0, 2, 1};

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        u_int32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        u_int32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        u_int32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        u_int32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}

//Justine Cherry - Blindbox
void make_uint_array_from_blob(int* dest, unsigned char* blob, u_int32_t bloblen)
{
  memset(dest, 0, bloblen * 8);
  int x;
  for (x = 0; x < bloblen; x++)
  {
    unsigned char thisblob = blob[x];
#ifdef BIGENDHEYO
    dest[x * 8] = (0x80 & thisblob) == 0x80 ? 1 : 0;
    dest[x * 8 + 1] = (0x40 & thisblob) == 0x40 ? 1 : 0;
    dest[x * 8 + 2] = (0x20 & thisblob) == 0x20 ? 1 : 0;
    dest[x * 8 + 3] = (0x10 & thisblob) == 0x10 ? 1 : 0;
    dest[x * 8 + 4] = (0x08 & thisblob) == 0x08 ? 1 : 0;
    dest[x * 8 + 5] = (0x04 & thisblob) == 0x04 ? 1 : 0;
    dest[x * 8 + 6] = (0x02 & thisblob) == 0x02 ? 1 : 0;
    dest[x * 8 + 7] = (0x01 & thisblob) == 0x01 ? 1 : 0;
#else
    dest[x * 8 + 7] = (0x80 & thisblob) == 0x80 ? 1 : 0;
    dest[x * 8 + 6] = (0x40 & thisblob) == 0x40 ? 1 : 0;
    dest[x * 8 + 5] = (0x20 & thisblob) == 0x20 ? 1 : 0;
    dest[x * 8 + 4] = (0x10 & thisblob) == 0x10 ? 1 : 0;
    dest[x * 8 + 3] = (0x08 & thisblob) == 0x08 ? 1 : 0;
    dest[x * 8 + 2] = (0x04 & thisblob) == 0x04 ? 1 : 0;
    dest[x * 8 + 1] = (0x02 & thisblob) == 0x02 ? 1 : 0;
    dest[x * 8] = (0x01 & thisblob) == 0x01 ? 1 : 0;

#endif

  }
}
