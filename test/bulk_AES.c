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
#include <gnutls/gnutls.h>
#include <gnutls/x509.h>
#include <gnutls/crypto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "justGarble.h"

#include "torabuild.h"

int main() {
  srand(time(NULL));
  int rounds = 10;
  int n = 128 + (128 * (rounds + 1));
  int m = 128;

  GarbledCircuit circuit;
  block inputLabels[2 * n];
  block outputMap[2 * m]; //JMS: Change from m to 2 * m
  int inputs[n];
  int i, j;

  //JMS: Replace timing with check for correctness
  block finalOutput[m];
  block extractedLabels[n];

  //M4K9VgH0xRqf84t6j6nlMg==
  unsigned char userkey[16] = {
		0x33, 0x82, 0xBD, 0x56, 0x01, 0xF4, 0xC5, 0x1A,
		0x9F, 0xF3, 0x8B, 0x7A, 0x8F, 0xA9, 0xE5, 0x32
	}; //need 16 bytes

  //input for 0123456789ABCDE <padd>
	unsigned char plaintext[16] = {
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x41, 0x42, 0x43, 0x44, 0x45, 0x01
	};

  /*
  //-----------------KEY SETUP-----------------------------------------------
	block *keysched = (block *) malloc(sizeof(block)*11);//11 round keys

	unsigned char *testchar = (char *) userkey_bcast;
	block *userkey = (block *) userkey_bcast;
	size_t outlen;
	AES_128_Key_Expansion(userkey, keysched);
	printf("TX Key (base64): %s\n",base64_encode(testchar,16,&outlen));
	print128_num(*userkey);
	printf("Keyschedule round keys :\n");
	printblock_arr(keysched, 11);
	//-----------------------------------------------------------------------


	block *plain = (block *) plaintext;
	printf("Plaintext (base64): %s\n",base64_encode(plaintext,16,&outlen));

	//first 128bit is from bob, rest is the keyschedule from alice
	block2bitstream(plain, inputs , 0 ,0 , 1); //128bits from 0-128
	for(i=1;i<rounds+2;i++){
		block2bitstream(keysched, inputs,(i-1)*128,i * 128, 1);
	}

	printf("Inputs:\n");
	printbitstream(inputs, 12,128);
  //print input key as well as the keyschedules----------------------------
  */
  struct timeval total_start;
  struct timeval total_stop;
  clock_t start,end;
  double cpu_time_used;

  build_JustineAES(&circuit);

  int total_instances = 3000;

  gettimeofday(&total_start, NULL);
  start = clock();
  for(i=0;i<total_instances;i++){

    garbleCircuit(&circuit, inputLabels, outputMap);
    setup_AESInput(inputs, plaintext, userkey, n);
    extractLabels(extractedLabels, inputLabels, inputs, n);
    evaluate(&circuit, extractedLabels, finalOutput);
    int outputVals[m];
    memset(outputVals, 0, sizeof(int) * m);
    mapOutputs(outputMap, finalOutput, outputVals, m);

    //printf("AES output (ciphertext):\n");
  	//printbitstream(outputVals,1,m);
  }
  end = clock();
  gettimeofday(&total_stop, NULL);

  cpu_time_used = ((double) (end-start))/ CLOCKS_PER_SEC;

  //printf("Total time for %d garbling and eval : %lu us\n", total_instances  ,total_stop.tv_usec - total_start.tv_usec);
  //printf("In seconds : %f\n",(total_stop.tv_usec - total_start.tv_usec) / 1000000.0 );
  printf("Total time for %d garbling and eval : %f s\n",total_instances,cpu_time_used);

  return 0;
}
