#include <iostream>

#include "Huffman.h"

int main()
{
	try {
		Huffman<char>::compressed huf;
		huf.load("D:\\Uri Raz\\nonexistent.txt");
	}
	catch (std::filesystem::filesystem_error) {
		std::cout << "OK: File not found\n";
	}

	try {
		Huffman<char>::compressed huf;
		huf.load("D:\\Uri Raz\\empty.txt");
	}
	catch (std::filesystem::filesystem_error) {
		std::cout << "OK: Corrupt file\n";
	}

	std::cout << '\n';

	std::string data;

	FILE* inFile = fopen("D:\\Uri Raz\\Equipment.txt", "r");
//	FILE* inFile = fopen("D:\\Uri Raz\\C++ Projects\\Huffman\\Tooth filtered from 116 photos.txt", "r");
	int c;
	while ((c = std::fgetc(inFile)) != EOF)
		data += c;
	fclose(inFile);

	Huffman<char> huf;
	auto compressed = huf.compress(data);
//	compressed.save("D:\\Uri Raz\\C++ Projects\\Huffman\\compressed.huf");
//	Huffman<char>::compressed chuchu;
//	chuchu.load("D:\\Uri Raz\\C++ Projects\\Huffman\\compressed.huf");
	auto str = huf.decompress(compressed);
	std::cout << str;
}