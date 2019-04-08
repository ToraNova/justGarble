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

#include <cstdlib>
#include <ENCRYPTO_utils/connection.h>

#include "ot/alsz-ot-ext-snd.h"
#include "ot/alsz-ot-ext-rec.h"
#include "ot/xormasking.h"

#include "ENCRYPTO_utils/socket.h"
#include <ENCRYPTO_utils/typedefs.h>
#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/cbitvector.h>
#include <ENCRYPTO_utils/rcvthread.h>
#include <ENCRYPTO_utils/sndthread.h>
#include <ENCRYPTO_utils/channel.h>
#include <ENCRYPTO_utils/timer.h>
#include <ENCRYPTO_utils/parse_options.h>

#include <vector>
#include <sys/time.h>

#include <limits.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

//pthread_mutex_t CLock::share_mtx = PTHREAD_MUTEX_INITIALIZER;



BOOL Cleanup()
{
	delete sndthread;

	//rcvthread->Wait();

	delete rcvthread;

	//std::cout << "Cleaning" << std::endl;
	//std::cout << "done" << std::endl;
	return true;
}


void InitOTSender(const std::string& address, const int port, crypto* crypt, CLock *glock)
{
#ifdef OTTiming
	timespec np_begin, np_end;
#endif
	m_nPort = (uint16_t) port;
	m_nAddr = &address;

	//Initialize values
	Init(crypt);

	//Server listen
	m_Socket = Listen(address, port);
	if (!m_Socket) {
		std::cerr << "Listen failed on " << address << ":" << port << "\n";
		std::exit(1);
	}

	sndthread = new SndThread(m_Socket.get(), glock);
	rcvthread = new RcvThread(m_Socket.get(), glock);

	rcvthread->Start();
	sndthread->Start();

	switch(m_eProt) {
		case ALSZ: sender = new ALSZOTExtSnd(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks); break;
		//case IKNP: sender = new IKNPOTExtSnd(crypt, rcvthread, sndthread); break;
		//case NNOB: sender = new NNOBOTExtSnd(crypt, rcvthread, sndthread); break;
		//case KK: sender = new KKOTExtSnd(crypt, rcvthread, sndthread); break;
		default: sender = new ALSZOTExtSnd(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks); break;
	}

	if(m_bUseMinEntCorAssumption)
		sender->EnableMinEntCorrRobustness();
	sender->ComputeBaseOTs(m_eFType);
}

void InitOTReceiver(const std::string& address, const int port, crypto* crypt, CLock *glock)
{
	m_nPort = (uint16_t) port;
	m_nAddr = &address;

	//Initialize values
	Init(crypt);

	//Client connect
	m_Socket = Connect(address, port);
	if (!m_Socket) {
		std::cerr << "Connect failed on " << address << ":" << port << "\n";
		std::exit(1);
	}

	sndthread = new SndThread(m_Socket.get(), glock);
	rcvthread = new RcvThread(m_Socket.get(), glock);

	rcvthread->Start();
	sndthread->Start();

	switch(m_eProt) {
		case ALSZ: receiver = new ALSZOTExtRec(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks); break;
		//case IKNP: receiver = new IKNPOTExtRec(crypt, rcvthread, sndthread); break;
		//case NNOB: receiver = new NNOBOTExtRec(crypt, rcvthread, sndthread); break;
		//case KK: receiver = new KKOTExtRec(crypt, rcvthread, sndthread); break;
		default: receiver = new ALSZOTExtRec(crypt, rcvthread, sndthread, m_nBaseOTs, m_nChecks); break;
	}


	if(m_bUseMinEntCorAssumption)
		receiver->EnableMinEntCorrRobustness();
	receiver->ComputeBaseOTs(m_eFType);
}


BOOL ObliviouslySend(CBitVector** X, int numOTs, int bitlength, uint32_t nsndvals,
		snd_ot_flavor stype, rec_ot_flavor rtype, crypto* crypt)
{
	bool success = FALSE;

	m_Socket->ResetSndCnt();
	m_Socket->ResetRcvCnt();
	timespec ot_begin, ot_end;

	clock_gettime(CLOCK_MONOTONIC, &ot_begin);
	// Execute OT sender routine
	success = sender->send(numOTs, bitlength, nsndvals, X, stype, rtype, m_nNumOTThreads, m_fMaskFct);
	clock_gettime(CLOCK_MONOTONIC, &ot_end);

#ifndef BATCH
	printf("Time spent:\t%f\n", getMillies(ot_begin, ot_end) + rndgentime);
	std::cout << "Sent:\t\t" << m_Socket->getSndCnt() << " bytes" << std::endl;
	std::cout << "Received:\t" << m_Socket->getRcvCnt() <<" bytes" << std::endl;
#else
	std::cout << getMillies(ot_begin, ot_end) + rndgentime << "\t" << m_Socket->getSndCnt() << "\t" << m_Socket->getRcvCnt() << std::endl;
#endif


	return success;
}

BOOL ObliviouslyReceive(CBitVector* choices, CBitVector* ret, int numOTs, int bitlength, uint32_t nsndvals,
		snd_ot_flavor stype, rec_ot_flavor rtype, crypto* crypt)
{
	bool success = FALSE;

	m_Socket->ResetSndCnt();
	m_Socket->ResetRcvCnt();


	timespec ot_begin, ot_end;
	clock_gettime(CLOCK_MONOTONIC, &ot_begin);
	// Execute OT receiver routine
	success = receiver->receive(numOTs, bitlength, nsndvals, choices, ret, stype, rtype, m_nNumOTThreads, m_fMaskFct);
	clock_gettime(CLOCK_MONOTONIC, &ot_end);

#ifndef BATCH
	printf("Time spent:\t%f\n", getMillies(ot_begin, ot_end) + rndgentime);

	std::cout << "Sent:\t\t" << m_Socket->getSndCnt() << " bytes" << std::endl;
	std::cout << "Received:\t" << m_Socket->getRcvCnt() <<" bytes" << std::endl;
#else
	std::cout << getMillies(ot_begin, ot_end) + rndgentime << "\t" << m_Socket->getSndCnt() << "\t" << m_Socket->getRcvCnt() << std::endl;
#endif


	return success;
}

int main(){
  return 0;
}
