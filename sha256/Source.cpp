#include<stdlib.h>
#include<iostream>
#include<ctime>
#include<random>
#include<string>

typedef unsigned short	   int uint8;
typedef unsigned		   int uint16;
typedef unsigned long	   int uint32;
typedef unsigned long long int uint64;

#define INIT_VECTORS				8u
#define NUMBER_OF_CONSTANTS			64u
#define NUM_OF_ZERO_BYTES			56u
#define MESSAGE_LENGTH_BYTES		8u
#define	UINT8_FIRST_BIT_1			128u
#define BITS_IN_UINT32				32u
#define BYTES_IN_MESSAGE			64u
#define FIRST_32_BIT_WORDS			16u
#define BITS_IN_BYTE				8u
#define NUMBER_OF_32_BIT_WORDS		64u
#define UINT8_MASK					0xFF

const uint32 initHash[INIT_VECTORS] = {
	0x5be0cd19,		// 0
	0x1f83d9ab,		// 1
	0x9b05688c,		// 2
	0x510e527f,		// 3
	0xa54ff53a,		// 4
	0x3c6ef372,		// 5
	0xbb67ae85,		// 6
	0x6a09e667		// 7
};

const uint32 constants[NUMBER_OF_CONSTANTS] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

using namespace std;

static inline uint32 rotr(uint32 x, uint8 places);
static inline uint32 ch(uint32 x, uint32 y, uint32 z);
static inline uint32 maj(uint32 x, uint32 y, uint32 z);
static inline uint32 sum1(uint32 x);
static inline uint32 sum2(uint32 x);
static inline uint32 o1(uint32 x);
static inline uint32 o2(uint32 x);

string gen_random(const int len) {

	string tmp_s;
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	tmp_s.reserve(len);

	for (int i = 0; i < len; ++i) {
		tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return tmp_s;
}

void main() {
	string message1 = "Proof of work! ";
	string message;
	string randAppend;
	uint8 word[BYTES_IN_MESSAGE];
	uint8 stop;
	uint64 messageLen = 0u;
	uint8 i, j;
	uint32 words[NUMBER_OF_32_BIT_WORDS];
	uint32 tempInit[INIT_VECTORS];
	uint32 t1, t2;
	uint32 result[INIT_VECTORS];
	int beginTime;
	uint32 counter = 0u;
	uint8 head = 0u;

	srand((int)clock());

	tempInit[7u] = 0xFFFFFFFF;
	/*start the "stopwatch"*/
	beginTime = (int)clock();

	//for (counter = 0u; counter < 1000000u; ++counter)
	while(((initHash[7u] + tempInit[7u]) & 0xFFFFFFF0) != 0u) {
		counter++;
		randAppend = gen_random(16);
		message = message1;
		message.append(randAppend);
		messageLen = message.length();

		// initialize words to 0
		for (i = 0u; i < NUMBER_OF_32_BIT_WORDS; i++) {
			words[i] = 0u;
		}

		// intialize temp values of init vector
		for (i = 0; i < INIT_VECTORS; i++) {
			tempInit[i] = initHash[i];
		}

		// copy message
		for (i = 0u; i < messageLen; i++) {
			word[i] = message[i];
		}

		// append first 1
		word[messageLen] = UINT8_FIRST_BIT_1;

		// append zeros
		for (i = messageLen + 1u; i < NUM_OF_ZERO_BYTES; i++) {
			word[i] = 0u;
		}

		// add message length
		for (i = 0u; i < MESSAGE_LENGTH_BYTES; i++) {
			word[i + NUM_OF_ZERO_BYTES] = (messageLen * BITS_IN_BYTE) & ((uint64)UINT8_MASK << (8u * (7u - i)));
		}
		// message ready

		head = 0u;
		// intialize temp values of init vector
		for (i = 0; i < INIT_VECTORS; ++i) {
			tempInit[i] = initHash[i];
		}

		// first 16 32bit words
		for (i = 0u; i < FIRST_32_BIT_WORDS; ++i) {
			words[i] = ((uint32)word[i * 4u] << 24u) +
					   ((uint32)word[i * 4u + 1u] << 16u) +
					   ((uint32)word[i * 4u + 2u] << 8u) +
					   (word[i * 4u + 3u]);
		}

		// calculate rest of the words
		for (i = FIRST_32_BIT_WORDS; i < NUMBER_OF_32_BIT_WORDS; ++i) {
			words[i] = words[i - 16u] + o1(words[i - 15u]) + words[i - 7u] + o2(words[i - 2u]);
		}

		/*for (i = 0u; i < NUMBER_OF_32_BIT_WORDS; i++) {
			cout << i << ":\t";
			cout << hex << words[i] << endl;																		// cout
		}*/

		// compression
		for (i = 0u; i < NUMBER_OF_32_BIT_WORDS; ++i) {
			// calculate temp 1
			t1 = sum2(tempInit[(head + 3u) & 0b111]) + ch(tempInit[(head + 3u) & 0b111], tempInit[(head + 2u) & 0b111], tempInit[(head + 1u) & 0b111]) + tempInit[head] + constants[i] + words[i];
			// calculate temp 2
			t2 = sum1(tempInit[(head + 7u) & 0b111]) + maj(tempInit[(head + 7u) & 0b111], tempInit[(head + 6u) & 0b111], tempInit[(head + 5u) & 0b111]);

			/*// move temp values 1 place higher
			for (j = 0u; j < 7u; j++) {
				tempInit[7u - j] = tempInit[6u - j];
			}*/


			// assign t1 + t2 to first temp value
			tempInit[head] = t1 + t2;
			// add t1 to temp value [4u]
			tempInit[(head + 4u) & 0b111] += t1;

			++head;
			head &= 0b111;
		}
	}


	/*show the execution time*/
	cout << "time: " << (int)clock() - beginTime << endl;

	for (i = 0u; i < INIT_VECTORS; i++) {
		result[i] = initHash[7u - i] + tempInit[7u - i];
		cout << hex << result[i];																				// cout
	}

	cout << endl;
	cout << dec << counter << endl;
	cout << message << endl;

	cin >> stop;
}


static inline uint32 rotr(uint32 x, uint8 places) {
	return (x >> places) | (x << (BITS_IN_UINT32 - places));
}



static inline uint32 ch(uint32 x, uint32 y, uint32 z) {
	return (x & y) ^ (~x & z);
}

static inline uint32 maj(uint32 x, uint32 y, uint32 z) {
	return (x & y) ^ (x & z) ^ (y & z);
}



static inline uint32 sum1(uint32 x) {
	return rotr(x, 2u) ^ rotr(x, 13u) ^ rotr(x, 22u);
}

static inline uint32 sum2(uint32 x) {
	return rotr(x, 6u) ^ rotr(x, 11u) ^ rotr(x, 25u);
}

static inline uint32 o1(uint32 x) {
	return rotr(x, 7u) ^ rotr(x, 18u) ^ (x >> 3u);
}

static inline uint32 o2(uint32 x) {
	return rotr(x, 17u) ^ rotr(x, 19u) ^ (x >> 10u);
}