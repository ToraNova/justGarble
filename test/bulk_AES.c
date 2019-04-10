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
  int i, j, x;

  //JMS: Replace timing with check for correctness
  block finalOutput[m];
  block extractedLabels[n];
  int outputVals[m];
  int aes_output_int[m];

  AES_KEY key;
  unsigned char output_aes[16];

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

  //evaluation vars
  struct timeval total_start;
  struct timeval total_stop;
  clock_t start,end;
  double garble_eval_time, rulegen_time;


  int total_instances = 6000;

  build_JustineAES(&circuit);



  start = clock();
  for(i=0;i<total_instances;i++){

    garbleCircuit(&circuit, inputLabels, outputMap);
    setup_AESInput(inputs, plaintext, userkey, &key, n);
    extractLabels(extractedLabels, inputLabels, inputs, n);
    evaluate(&circuit, extractedLabels, finalOutput);

    memset(outputVals, 0, sizeof(int) * m);
    mapOutputs(outputMap, finalOutput, outputVals, m);

    //printf("AES output (ciphertext):\n");
  	//printbitstream(outputVals,1,m);
  }
  end = clock();
  garble_eval_time = ((double) (end-start))/ CLOCKS_PER_SEC;

  start = clock();
  for(i=0;i<total_instances;i++){
    setup_AESInput(inputs, plaintext, userkey, &key, n);
    AES_encrypt(plaintext, output_aes, &key);
    make_uint_array_from_blob(aes_output_int, output_aes, 16);
  }
  end = clock();
  rulegen_time = ((double) (end-start))/ CLOCKS_PER_SEC;

  printf("AES output (ciphertext):\n");
  printf("Garbled circuit :");
  for(x = 0; x < 128; x++) printf("%u", outputVals[x]);
  printf("\n");
  printf("Vanilla AES     :");
  for(x = 0; x < 128; x++) printf("%u", aes_output_int[x]);
  printf("\n\n");

  //printf("Total time for %d garbling and eval : %lu us\n", total_instances  ,total_stop.tv_usec - total_start.tv_usec);
  //printf("In seconds : %f\n",(total_stop.tv_usec - total_start.tv_usec) / 1000000.0 );
  printf("Total time for %d AES circuit test :\n Garbling and eval : %f s\n Rulegen : %f s\n",
    total_instances,garble_eval_time, rulegen_time);

  return 0;
}
