#include <iostream>

#include "Huffman.h"

int main()
{
	std::string data;

	FILE* inFile = fopen("<insert file name here>", "r");
	int c;
	while ((c = std::fgetc(inFile)) != EOF)
		data += c;
	fclose(inFile);

	Huffman<char> huf;
	auto compressed = huf.compress(data);
	compressed.save("compressed.huf");
	Huffman<char>::compressed compressedFile;
	compressedFile.load("compressed.huf");
	auto str = huf.decompress(compressed);
	std::cout << str;
}