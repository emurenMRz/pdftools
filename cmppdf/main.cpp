#include "pdf.h"
#include <iostream>

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		puts("usage > cmppdf [first.pdf] [second.pdf]");
		return 1;
	}

	try
	{
		auto first = PDF::Document(argv[1]);
		std::cout << first;
		auto second = PDF::Document(argv[2]);
		std::cout << second;

		first.diff(std::cout, second);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
