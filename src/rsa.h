#ifndef RSA_INCLUDED
#define RSA_INCLUDED

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <cassert>
#include <string>
#include <cstring>
#include <cstdlib>
#include "gmp.h"
#include "gmpxx.h"

using namespace std;

void encode(ifstream &in, ofstream &out, ifstream &key, char const command);
void help();

#endif