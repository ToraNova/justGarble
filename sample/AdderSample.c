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

// chia_jason96@live.com - test case on justGarble, simple adder problem

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "justGarble.h"
#include "gates.h"

#define CIRCUIT_FILENAME "./4bitAdder_nocarry" //output filename
#define SENDING_TEST
//essential here as it is the pointer to the output
int *final;

void fullAdder4bit(GarbledCircuit *buildTarget){

	//attempt to build a 4bit adder

	// trying to build the following garble
	// f(1,2,3,4) = 1 && 2 && 3 && 4
	srand(time(NULL));
	GarblingContext garblingContext;
	int n = 8; //9 inputs, a0-a3 and b0-b3 with carry in and out
	int m = 4; //out0 - out3
	int q = 20;												//upper bound. no. of gates
	int r = 30;												//upper bound. no. of wires
	int i; //counter

	block labels[2 * n];
	block outputbs[m]; //output block
	OutputMap outputMap = outputbs;
	InputLabels inputLabels = labels;

	final = (int *)malloc(4*sizeof(int));
	//setup input array
	int inp[n];
	countToN(inp,n);
	createInputLabels(labels, n);
	createEmptyGarbledCircuit(buildTarget, n, m, q, r, inputLabels);
	startBuilding(buildTarget, &garblingContext);

	ADDCircuit(buildTarget, &garblingContext, n, inp, final );
	finishBuilding(buildTarget, &garblingContext, outputMap, final);

	//hypothesis : we do not need to care about the labels generated here
}

int addtest(int *in, int *out, int s){
	//do not initialize in and out again,
	//they are already initialized from the check
	//routine
	int i;
	int c;
	c = 0;
	for(i=0;i<s/2;i++){
		out[i] = in[i]^in[i+4]^c;
		c = (in[i]&in[i+4]) | (c & (in[i]^in[i+4]));
	}
	return 0;
}

int main() {
	int n = 8;
	int m = 4; //observation suggests that we need at least 2 outputs

	//operational declares
	//operational declares
	GarbledCircuit in4add;
	GarbledCircuit re4add;
	block inputLabels[2 * n];
	block outputMap[2*m];
	//for transmission and eval
	block extractedLabels[n]; //the real labels being sent over
	block computedOutputMap[m];
	int outputVals[m];

	int a0=1,a1=1,a2=0,a3=0;
	int b0=1,b1=1,b2=0,b3=0;//THIS IS DONE OVER OT, alice does not know

	//SEND side, build the circuit and extract the garbled circuit
	//both sides will have the same topology
	//thus the circuit can be built individually first
	fullAdder4bit(&in4add);
	garbleCircuit( &in4add, inputLabels, outputMap);

#ifdef SENDING_TEST
	fullAdder4bit(&re4add);

	int inputs[8] = {a0,a1,a2,a3,b0,b1,b2,b3}; //TODO, how to obliv xfer here ?
	extractLabels(extractedLabels,inputLabels,inputs,n);
	//the extractLabels is sent over to bob, and the garbled table

	re4add.garbledTable = in4add.garbledTable; //simulate sending
	re4add.globalKey = in4add.globalKey; //key must be sent over as well
	evaluate(&re4add, extractedLabels, computedOutputMap); //bob evals
	//the output map is then sent back to alice

	//alice maps the outputs
	mapOutputs(outputMap, computedOutputMap, outputVals, m);
	printbin_arr("Final output:",outputVals,m);

#else

	//alice tests the circuit locally to ensure correctness
	checkCircuit(&in4add, inputLabels, outputMap, addtest);

#endif

	return 0;
}
