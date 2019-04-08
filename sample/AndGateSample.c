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

// chia_jason96@live.com - test case on justGarble, simple AND problem


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "justGarble.h"
#include "gates.h"

#define CIRCUIT_FILENAME "./simple2And1" //output filename
#define SENDING_TEST

//global int
int *final;

void simpleANDbuild(GarbledCircuit *buildTarget){

	// trying to build the following garble
	// f(1,2,3,4) = 1 && 2 && 3 && 4

	srand(time(NULL));

	GarblingContext garblingContext;

	int n = 2; //2 inputs, Alice and Bob's value (a1,a2,b3,b4)
	int m = 1; //1 ouput, true or false

	int q = 1;												//3 2input AND gates
	int r = 3;												//7 wires

	int i; //counter

	block labels[2 * n];
	block outputbs[m]; //output block
	OutputMap outputMap = outputbs;
	InputLabels inputLabels = labels;

	final = (int *)malloc(1*sizeof(int));

	//setup input array
	int inp[n];
	countToN(inp,n);
	createInputLabels(labels, n);
	createEmptyGarbledCircuit(buildTarget, n, m, q, r, inputLabels);
	startBuilding(buildTarget, &garblingContext);

	//this works for now
	ANDCircuit(buildTarget, &garblingContext, n, inp, final);
	finishBuilding(buildTarget, &garblingContext, outputMap, final);

	//hypothesis : we do not need to care about the labels generated here
}

int andtest(int *in, int *out, int s){
	//do not initialize in and out again,
	//they are already initialized from the check
	//routine
	int i;
	out[0] = 1;
	for(i=0;i<s;i++){
		out[0] = out[0] & in[i];
	}
	return 0;
}

int main() {
	int n = 2;
	int m = 1; //observation suggests that we need at least 2 outputs

	//operational declares
	GarbledCircuit in4and;
	GarbledCircuit re4and;
	block inputLabels[2 * n];
	block outputMap[2*m];
	//for transmission and eval
	block extractedLabels[n]; //the real labels being sent over
	block computedOutputMap[m];
	int outputVals[m];

	int choice_alice = 1;
	int choice_bob = 0; //THIS IS DONE OVER OT, alice does not know

	//SEND side, build the circuit and extract the garbled circuit
	//both sides will have the same topology
	//thus the circuit can be built individually first
	simpleANDbuild(&in4and);
	garbleCircuit( &in4and, inputLabels, outputMap);

#ifdef SENDING_TEST
	simpleANDbuild(&re4and);

	int inputs[2] = {choice_alice,choice_bob};
	extractLabels(extractedLabels,inputLabels,inputs,n);
	//the extractLabels is sent over to bob, and the garbled table

	re4and.garbledTable = in4and.garbledTable; //simulate sending
	re4and.globalKey = in4and.globalKey; //key must be sent over as well
	evaluate(&re4and, extractedLabels, computedOutputMap); //bob evals
	//the output map is then sent back to alice

	//alice maps the outputs
	printblock_arr(inputLabels,2*n);
	printblock_arr(extractedLabels,n);
	printblock_arr(outputMap,2*m);
	printblock_arr(computedOutputMap,m);
	mapOutputs(outputMap, computedOutputMap, outputVals, m);
	printbin_arr("Final output:",outputVals,m);

#else

	//alice tests the circuit locally to ensure correctness
	checkCircuit(&in4and, inputLabels, outputMap, andtest);

#endif

	return 0;
}
