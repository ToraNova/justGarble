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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "justGarble.h"

#define AES_CIRCUIT_FILE_NAME "./aesCircuit" //output filename

void buildAESCircuit(GarbledCircuit *buildTarget) {
	//set seed for RNG
	int *final;

	//declares garble circuit
	GarblingContext garblingContext;

	//AES round limit
	int roundLimit = 10; 									//10round AES
	int n = 128 * (roundLimit + 1); 			//number of inputs 11x128
	int m = 128;													//number of outputs
	int q = 50000;												//number of gates (upper bound)
	int r = 50000;												//?

	int inp[n];															//input array ? (1)

	int addKeyInputs[n * (roundLimit + 1)]; // key input ? (4b) (11x128) x (11)
	int addKeyOutputs[n];										// (5)
	int subBytesOutputs[n];									// (2)
	int shiftRowsOutputs[n];								// (3)
	int mixColumnOutputs[n];								// (4)
	int round;															// round counter
	block labels[2 * n];
	block outputbs[m]; //output block
	OutputMap outputMap = outputbs;
	InputLabels inputLabels = labels;
	int i;

	countToN(inp, n); //inp goes from 0-127 for each element (initializes) (numbering the input wires)
	createInputLabels(labels, n);
	createEmptyGarbledCircuit(buildTarget, n, m, q, r, inputLabels);
	printf("ERROR_DETLINE...e3\n");
	startBuilding(buildTarget, &garblingContext);

	countToN(addKeyInputs, 256); //addkeyInputs goes from 0-255 //numbering the addkey input wires

	for (round = 0; round < roundLimit; round++) {
		printf("ERROR_DETLINE...e4d\n");
		AddRoundKey(buildTarget, &garblingContext, addKeyInputs,
				addKeyOutputs); //create the addroundkey circuits
		printf("ERROR_DETLINE...e5d\n");
		for (i = 0; i < 16; i++) {
			SubBytes(buildTarget, &garblingContext, addKeyOutputs + 8 * i,
					subBytesOutputs + 8 * i); //perform sub-bytes
		}
		ShiftRows(buildTarget, &garblingContext, subBytesOutputs,
				shiftRowsOutputs); //perform shifting
		for (i = 0; i < 4; i++) {
			if (round == roundLimit - 1)
				MixColumns(buildTarget, &garblingContext,
						shiftRowsOutputs + i * 32, mixColumnOutputs + 32 * i); //for each column, mix it with a vector
		}
		for (i = 0; i < 128; i++) {
			addKeyInputs[i] = mixColumnOutputs[i]; //setup the next round
			addKeyInputs[i + 128] = (round + 2) * 128 + i;
		}

	}

	final = mixColumnOutputs;
	finishBuilding(buildTarget, &garblingContext, outputMap, final);
	//writeCircuitToFile(buildTarget, AES_CIRCUIT_FILE_NAME);
}

int main() {
	srand(time(NULL));
	printf("Start eval...1\n");
	//int keysize = 128;

	//param declares
	int rounds = 10;
	int n = 128 + (128 * rounds);
	int m = 128;

	//operational declares
	GarbledCircuit aesCircuit_send;
	GarbledCircuit aesCircuit_recv;
	GarbledTable garbledTable_tx;
	block inputLabels[2 * n];
	block outputMap[m];
	int i, j;

	//evaluation declares
	//perform TESTING on garbling TIMES amount (TIMES is 10)
	int timeGarble[TIMES];
	int timeEval[TIMES];
	double timeGarbleMedians[TIMES];
	double timeEvalMedians[TIMES];
	int tmptime0,tmptime1;

	printf("Declares...2\n");
	//SEND side, build the AES circuit and extract the garbled circuit
	//both sides will have the same topology
	//thus the circuit can be built first
	buildAESCircuit(&aesCircuit_send);
	
	//RECV side, receive the garbledTable
	//both sides will have the same topology
	//thus the circuit can be built first
	//buildAESCircuit(&aesCircuit_recv);

	printf("Finish topological building on send and recv circuits...3\n");

	//readCircuitFromFile( aesCircuit, AES_CIRCUIT_FILE_NAME);
	//writeCircuitToFile(&garbledCircuit, AES_CIRCUIT_FILE_NAME);

	//single instance test
	tmptime0 = garbleCircuit( &aesCircuit_send, inputLabels, outputMap);
	printf("Sender has garbled the AES circuit [%d]\n",tmptime0);

	//sending simulation
	//aesCircuit_recv.garbledTable = aesCircuit_send.garbledTable;

	tmptime1 = timedEval(&aesCircuit_recv, inputLabels);
	printf("Receiver has finished evaluating the AES circuit [%d]\n",tmptime1);

	//evaluation
	/*
	for (j = 0; j < TIMES; j++) {
		for (i = 0; i < TIMES; i++) {

			//garbling on sender side
			timeGarble[i] = garbleCircuit(&aesCircuit_send, inputLabels, outputMap);

			//sending simulation
			garbledTable_tx = aesCircuit_send->garbledTable;
			aesCircuit_recv->garbledTable = garbledTable_tx;

			//evaluating on receiver side
			timeEval[i] = timedEval(&aesCircuit_recv, inputLabels);
		}
		timeGarbleMedians[j] = ((double) median(timeGarble, TIMES))
				/ aesCircuit_send->q;
		timeEvalMedians[j] = ((double) median(timeEval, TIMES)) / aesCircuit_recv->q;
	}
	double garblingTime = doubleMean(timeGarbleMedians, TIMES);
	double evalTime = doubleMean(timeEvalMedians, TIMES);
	printf("%lf %lf\n", garblingTime, evalTime);
	*/
	return 0;
}
