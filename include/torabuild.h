
#ifndef TORABUILD_H_
#define TORABUILD_H_

/*
  Torabuild for standard circuit builders
  reduce size of main code
*/

#include "justGarble.h"
#include "aes.h"

//setup the input array (bitarray) with plaintext taking first 128bit, subsequently output from KS
void setup_AESInput(int *setup_target, unsigned char *plaintext, unsigned char *userkey, AES_KEY *k, int n);

#define BUILD_VERBOSE
void build_JustineAES(GarbledCircuit *buildTarget);

#endif
