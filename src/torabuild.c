
#include "torabuild.h"

void build_JustineAES(GarbledCircuit *buildTarget) {
  GarblingContext garblingContext;
  const char *circuitName = "justine_aes.gc";

  int roundLimit = 10;
  int n = 128 + 128 * (roundLimit + 1); //XOR the key 11 times
  int m = 128;
  int q = 500000; //Just an upper bound
  int r = 500000;

  //intermediaries
  int addKeyInputs[n * (roundLimit + 1)];
  int addKeyOutputs[n];
  int subBytesOutputs[n];
  int shiftRowsOutputs[n];
  int mixColumnOutputs[n];

  //labelling holders
  block labels[2 * n];
  block outputbs[2*m];
  OutputMap outputMap = outputbs;
  InputLabels inputLabels = labels;

  //counters and pointing vars
  int round;
  int i;
  int *final;
  long gates;

  //initialize input array 1-N
  int inp[n];
  countToN(inp, n);

  createInputLabels(labels, n);
  createEmptyGarbledCircuit(buildTarget, n, m, q, r, inputLabels);
  startBuilding(buildTarget, &garblingContext);

  countToN(addKeyInputs, n * 2);
  memset(subBytesOutputs, 0, sizeof(int) * 128);

  gates = garblingContext.gateIndex;
  AddRoundKey(buildTarget, &garblingContext, addKeyInputs,addKeyOutputs);

  for (round = 1; round < 11; round++) {

    for (i = 0; i < 16; i++) {
      JustineSBOX(buildTarget, &garblingContext, addKeyOutputs + 8 * i,
          subBytesOutputs + 8 * i);
    }

    ShiftRows(buildTarget, &garblingContext, subBytesOutputs,
        shiftRowsOutputs);

    for (i = 0; i < 4; i++) {
      if (round != roundLimit)
        JustineMixColumns(buildTarget, &garblingContext,
            shiftRowsOutputs + i * 32, mixColumnOutputs + 32 * i);
    }

    for (i = 0; i < 128; i++) {
      if(round != roundLimit) addKeyInputs[i] = mixColumnOutputs[i];
      else addKeyInputs[i] = shiftRowsOutputs[i];
      addKeyInputs[i + 128] = (round + 1) * 128 + i;
    }

    AddRoundKey(buildTarget, &garblingContext, addKeyInputs,addKeyOutputs);
  }

  //This part is consistent, just change the pointer assigned to final !
  final = addKeyOutputs;

  #ifdef BUILD_VERBOSE
  printf("Build Statistics for : %s\n No. of Gates : %lu\n No. of Wires : %lu\n",circuitName,
    garblingContext.gateIndex,garblingContext.wireIndex);
  #endif

  finishBuilding(buildTarget, &garblingContext, outputMap, final);
  writeCircuitToFile(buildTarget,circuitName);
}

void setup_AESInput(int *setup_target, unsigned char *plaintext, unsigned char *userkey, AES_KEY *inkey, int n){

  memset(inkey, 0, sizeof(AES_KEY));
  AES_set_encrypt_key(userkey, 128, inkey);

  make_uint_array_from_blob(setup_target, plaintext, 16);
  unsigned char* blob = inkey->rd_key;

  //printf("(%u) Input key...", n / 8 / 16);
  make_uint_array_from_blob(setup_target + 128, blob, n/8 - 16);
  //for(x = 0; x < n; x++) printf("%u", inputs[x]);
  //printf("\n\n");
}
