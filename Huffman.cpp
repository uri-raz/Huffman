#include <iostream>

#include "Huffman.h"

int main()
{
	std::string data;

	FILE* inFile = fopen("...\\File.txt", "r");
	int c;
	while ((c = std::fgetc(inFile)) != EOF)
		data += c;
	fclose(inFile);

	Huffman huf;
	auto compressed = huf.compress(data);
	compressed.save("...\\compressed.huf");
	Huffman::compressed chuchu;
	chuchu.load("...\\compressed.huf");
	auto str = huf.decompress(compressed);
	std::cout << str;
}