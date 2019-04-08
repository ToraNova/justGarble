#ifndef _AES_OT_H_
#define _AES_OT_H_

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

uint16_t m_nPort = 7766;
const std::string* m_nAddr;

BOOL Init(crypto* crypt);
BOOL Cleanup();

void InitOTSender(const std::string& address, const int port, crypto* crypt);
void InitOTReceiver(const std::string &address, const int port, crypto* crypt);

BOOL ObliviouslyReceive(CBitVector* choices, CBitVector* ret, int numOTs, int bitlength, uint32_t nsndvals, snd_ot_flavor stype, rec_ot_flavor rtype, crypto* crypt);
BOOL ObliviouslySend(CBitVector** X, int numOTs, int bitlength, uint32_t nsndvals, snd_ot_flavor stype, rec_ot_flavor rtype, crypto* crypt);

// Network Communication
std::unique_ptr<CSocket> m_Socket;
uint32_t m_nPID; // thread id
field_type m_eFType;
uint32_t m_nBitLength;
MaskingFunction* m_fMaskFct;

// Naor-Pinkas OT
//BaseOT* bot;
OTExtSnd *sender;
OTExtRec *receiver;

SndThread* sndthread;
RcvThread* rcvthread;

uint32_t m_nNumOTThreads;
uint32_t m_nBaseOTs;
uint32_t m_nChecks;

bool m_bUseMinEntCorAssumption;
ot_ext_prot m_eProt;

double rndgentime;


#endif
