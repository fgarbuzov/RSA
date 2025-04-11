#include "rsa.h"

int GetBit(const int n, const int i)
{
	return (n >> i) & 1;
}
int GetBitNumber(const mpz_t n)
{
	return mpz_sizeinbase(n, 2);
}
int GetByteNumber(const int bits_num)
{
	int bytes_num = bits_num / 8;
	if (bits_num % 8)
		++bytes_num;
	assert(bytes_num * 8 >= bits_num);
	return bytes_num;
}

void ExtractBits(mpz_t &uncoded_bits, int &uncoded_bits_num, char const * buf, streamsize const size)
{
	int byte_index = 0;
	mpz_t bits;
	mpz_init(bits);
	for (int i = 0; i < size * 8; ++i)
	{
    	assert(byte_index < size);
		if (i % 8 == 0 && i > 0)
		++byte_index;
		int bit = GetBit(buf[byte_index], i % 8);
		mp_bitcnt_t index = uncoded_bits_num + i;
		if (bit)
			mpz_setbit(bits, index);
		if (i == 0) 
			assert(byte_index == 0);
	}
	mpz_add(uncoded_bits, uncoded_bits, bits);
	uncoded_bits_num += (int)size * 8;
}

void EncodeBits(mpz_t &code, int &code_len, mpz_t &uncoded_bits, int &uncoded_bits_num, mpz_t const key, mpz_t const n, int const bit_num)
{
	int block_num = uncoded_bits_num / (bit_num - 1);
	code_len = 0;
	for (int j = 0; j < block_num; ++j)
	{
		mpz_t code_part;
		mpz_init(code_part);
		for (int i = 0; i < bit_num - 1; ++i)
		{
			mp_bitcnt_t index = i + j * (bit_num - 1);
			int bit = mpz_tstbit(uncoded_bits, index);
			if (bit)
				mpz_setbit(code_part, i);
		}
		mpz_powm(code_part, code_part, key, n);			// code_part = code_part^key mod n
		mpz_mul_2exp(code_part, code_part, j*bit_num);  // code_part = code_part << j*bit_num
		mpz_add(code, code, code_part);                 // code += code_part
		code_len += bit_num;
	}
	mpz_fdiv_q_2exp(uncoded_bits, uncoded_bits, block_num * (bit_num - 1));
	uncoded_bits_num -= (block_num * (bit_num - 1));
	assert(uncoded_bits_num >= 0);
	assert(uncoded_bits_num < bit_num - 1);
}

void DecodeBits(mpz_t &code, int &code_len, mpz_t &uncoded_bits, int &uncoded_bits_num, mpz_t const key, mpz_t const n, int const bit_num)
{
	int block_num = uncoded_bits_num / bit_num;
	code_len = 0;
	for (int j = 0; j < block_num; ++j)
	{
		mpz_t code_part;
		mpz_init(code_part);
		for (int i = 0; i < bit_num; ++i)
		{
			mp_bitcnt_t index = i + j * bit_num;
			int bit = mpz_tstbit(uncoded_bits, index);
			if (bit)
				mpz_setbit(code_part, i);
    	}

		mpz_powm(code_part, code_part, key, n);

		mpz_t max_num; mpz_init_set_ui(max_num, 1);
		mpz_mul_2exp(max_num, max_num, bit_num - 1);
		mpz_mod(code, code, max_num);
		mpz_mul_2exp(code_part, code_part, j*(bit_num-1));  // code_part = code_part << j*bit_num
		mpz_add(code, code, code_part);                    // code += code_part
		code_len += bit_num - 1;
	}
	mpz_fdiv_q_2exp(uncoded_bits, uncoded_bits, block_num * bit_num);
	uncoded_bits_num -= block_num * bit_num;
	assert(uncoded_bits_num >= 0);
	assert(uncoded_bits_num < bit_num);
}

void PutCodeIntoBuffer(char * buf, int &buflen, mpz_t const code, int const code_len)
{
	int byte_index = buflen / 8;
	for (int i = 0; i < code_len; ++i)
	{
		int bit = mpz_tstbit(code, i);
		buf[byte_index] += bit << (buflen % 8);
		buflen++;
		if (buflen % 8 == 0 && buflen > 0)
			byte_index++;
	}
}

void PrintCode(char * buf, int &buflen, ofstream &out)
{
	int full_bytes = GetByteNumber(buflen) - 1;

	if (buflen % 8 == 0)
		++full_bytes;
	for (int i = 0; i < full_bytes; ++i)
		out << buf[i];
	for (int i = full_bytes; i < 256; ++i)
		buf[i - full_bytes] = buf[i];

	buflen -= full_bytes * 8;
}

void PrintLastCode(char * buf, int &buflen, ofstream &out)
{
	int full_bytes = GetByteNumber(buflen) - 1, i;
	int bytes_num = full_bytes + 1;
  
	while (bytes_num > 0 && buf[bytes_num - 1] == 0)
		--bytes_num;

	if (buflen % 8 == 0)
		++full_bytes;
	for (int i = 0; i < bytes_num; ++i)
		out << buf[i];
	for (int i = bytes_num; i < 256; ++i)
		buf[i - bytes_num] = buf[i];

	if (full_bytes > bytes_num) 
		buflen -= full_bytes * 8;
	else
		buflen -= bytes_num * 8;

	if (buflen < 0)
		buflen = 0;
}

void CodeLastBits(mpz_t &code, int &code_len, mpz_t const uncoded_bits, int const uncoded_bits_num, mpz_t const key, mpz_t const n, int bit_num)
{
	for (int i = 0; i < uncoded_bits_num; ++i)
	{
		int bit = mpz_tstbit(uncoded_bits, i);
		if (bit)
			mpz_setbit(code, i);
	}

	if (code == 0)
	{
		code_len = 0;
		return;
	}

	mpz_powm(code, code, key, n);

	code_len = bit_num;
}

void encode(ifstream &in, ofstream &out, ifstream &keyfile, char const command)
{
	mpz_t k, n; // n - modulo
	mpz_init(k); mpz_init(n);
	keyfile >> k >> n;

	int bits_num  = GetBitNumber(n);
	int bytes_num = GetByteNumber(bits_num);
	char * buf      = (char*)calloc(bytes_num, sizeof(char));
	char * buf_code = (char*)calloc(256, sizeof(char));
	int buf_code_len = 0;
	mpz_t code, uncoded_bits;
	mpz_init(uncoded_bits);
	int uncoded_bits_num = 0, code_len;						                

	in.read(buf, bytes_num);
	do
	{
		if (in.gcount() == 0)
			break;
		ExtractBits(uncoded_bits, uncoded_bits_num, buf, in.gcount());
		mpz_init(code);
		switch (command)
		{
		case 'e': EncodeBits(code, code_len, uncoded_bits, uncoded_bits_num, k, n, bits_num); break;
		case 'd': DecodeBits(code, code_len, uncoded_bits, uncoded_bits_num, k, n, bits_num); break;
		}
		PutCodeIntoBuffer(buf_code, buf_code_len, code, code_len);
		mpz_clear(code);
		in.read(buf, bytes_num);
		if (in.gcount() == 0)
			break;
		else
			PrintCode(buf_code, buf_code_len, out);
	} while (1);

	mpz_init(code);
	CodeLastBits(code, code_len, uncoded_bits, uncoded_bits_num, k, n, bits_num);
	if (command == 'd')
	{
		mpz_t max_num; mpz_init_set_ui(max_num, 1);
		mpz_mul_2exp(max_num, max_num, bits_num - 1);
		mpz_mod(code, code, max_num);
		--code_len;
	}
	PutCodeIntoBuffer(buf_code, buf_code_len, code, code_len);
	PrintLastCode(buf_code, buf_code_len, out);

	free(buf); free(buf_code);
}

void help()
{
	cout << "This is help." << endl;
}