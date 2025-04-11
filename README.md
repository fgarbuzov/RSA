# RSA

This is a command line tool that allows to encrypt and decrypt files using the RSA algorithm. I created it during my second year at university as a part of a course project (discrete math course).

**Requires** GMP library to work with public and private keys.

**Usage**: `rsa.out <command> <key file> <source file> <output file>`

**Commands**:
- `e` encrypt the specified source file using the provided *public* key
- `d` decrypt the specified source file using the provided *secret* key

**Keys**

The key file must contain two numbers written in decimal and separated by a space. The first number is the modulus (the product of two prime numbers), and the second one is the exponent. The key length is limited to 1024 bits, accordingly, none of the numbers in the key files should exceed $2^{1024}-1$.
