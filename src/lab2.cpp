#include "rsa.h"

int main(int argc, char ** argv)
{
	if (argc != 5)
	{
		cerr << "ERROR: incorrect arguments" << endl;
		exit(1);
	}

	ifstream keyfile, in;
	ofstream out;
	keyfile.open(argv[2]); in.open(argv[3]); out.open(argv[4]);

	if (!keyfile || !in || !out)
	{
		cerr << "ERROR: failed to open input or output or key file" << endl;
		system("pause");
		exit(1);
	}

	char command = *(argv[1]);
	switch (command)
	{
	case 'e':
  	case 'd': encode(in, out, keyfile, command); break;
	default: cerr << "ERROR: incorrect command (first argument)" << endl;
	}

	keyfile.close(); in.close(); out.close();
	return 0;
}
