#pragma once

#include <map>
#include <queue>
#include <vector>
#include <cstddef>
#include <fstream>
#include <filesystem>
#include <system_error>
#include <unordered_map>

#include "bitVector.h"

template<typename T>
class Huffman
{
 public:
	struct freqPair {
		T		 val;
		uint32_t count;
	};

	class compressed {
	public:
		bitVector m_data;
		std::vector<freqPair> m_frequencies;

		bool isFull()  { return m_data.size() >  0; }
		bool isEmpty() { return m_data.size() == 0; }

		void save(const std::string& fileName) {
			std::ofstream outfile(fileName, std::ofstream::binary);

			uint64_t word = m_frequencies.size();
			outfile.write((const char*)&word, sizeof(uint64_t));

			word = m_data.size();
			outfile.write((const char*)&word, sizeof(uint64_t));

			for (auto p : m_frequencies)
				outfile.write((char*)&p, sizeof(freqPair));

			outfile.write((const char*)m_data.m_data.data(), m_data.m_data.size() * sizeof(uint64_t));

			outfile.close();

			if (!outfile.good())
				throw std::filesystem::filesystem_error(std::string("File " + fileName + " write failed"),
														std::error_code::error_code());
		}

		void load(const std::string& fileName) {
			m_data.clear();
			m_frequencies.clear();

			std::filesystem::path filePath(fileName);
			if (!std::filesystem::exists(filePath))
				throw std::filesystem::filesystem_error(std::string("File " + fileName + " not found"),
														std::error_code::error_code());

			auto fileSize = file_size(filePath);

			if (fileSize < 2 * sizeof(uint64_t)) [[unlikely]] {
				throw std::filesystem::filesystem_error(std::string("File " + fileName + " too short"),
														std::error_code::error_code());
			}

			std::ifstream infile(fileName, std::ofstream::binary);

			uint64_t numPairs;
			infile.read((char*)&numPairs, sizeof(uint64_t));

			uint64_t totalBits;
			infile.read((char*)&totalBits, sizeof(uint64_t));

			uint64_t numWords = totalBits / 64 + 1;

			if (fileSize < numPairs * sizeof(freqPair) + numWords * sizeof(uint64_t) + 2 * sizeof(uint64_t)) [[unlikely]] {
				throw std::filesystem::filesystem_error(std::string("File " + fileName + " too short"),
														std::error_code::error_code());
			}

			freqPair p;
			for (uint64_t i = 0; i < numPairs; i++) {
				infile.read((char*)&p, sizeof(freqPair));
				m_frequencies.push_back(p);
			}

			m_data.resize(totalBits);
			infile.read((char*)m_data.m_data.data(), numWords * sizeof(uint64_t));

			infile.close();

			if (!infile.good()) {
				m_data.clear();
				m_frequencies.clear();

				throw std::filesystem::filesystem_error(std::string("File " + fileName + " read failed"),
																	std::error_code::error_code());
			}
		}
	};

 private:
	template<typename T>
	class Tree
	{
	public:
		enum class status : std::uint8_t { Success, Duplicate, NotFound };

		struct node
		{
			node(T v = '\0', uint32_t c = 0, node* l = nullptr, node* r = nullptr) : val(v), count(c), left(l), right(r) {}

			T val;
			uint32_t count;
			node *left, *right;

			bool isLeaf() { return left == nullptr && right == nullptr; }
		};

		node* Root;
		
		Tree() : Root(nullptr) {}
		Tree(node* r) : Root(r) {}
		Tree(Tree&& rhs) noexcept : Root(rhs.Root) { rhs.Root = nullptr; }

		~Tree() {
			if (Root) {
				std::queue<node*> nodes;

				nodes.push(Root);
				Root = nullptr;
				while (nodes.size()) {
					auto head = nodes.front();
					nodes.pop();

					if (!head->isLeaf()) {
						nodes.push(head->left);
						nodes.push(head->right);
					}

					delete head;
				}
			}
		}

		std::map<T, bitVector> createDictionary() {
			bitVector prefix;
			std::map<T, bitVector> dictionary;

			if (Root)
				createDictionary(Root, prefix, dictionary);

			return dictionary;
		}

	private:
		void createDictionary(const node* Node,
							  bitVector& currPrefix,
							  std::map<T, bitVector>& dictionary) {
			if (Node->left) {
				currPrefix.push_back(false);
				createDictionary(Node->left, currPrefix, dictionary);
				currPrefix.pop_back();
				currPrefix.push_back(true);
				createDictionary(Node->right, currPrefix, dictionary);
				currPrefix.pop_back();
			}
			else
				dictionary.insert(std::make_pair(Node->val, currPrefix));
		}
	};

	class nodeCompare {
	public:
		bool operator()(const typename Tree<T>::node* lhs, const typename Tree<T>::node* rhs) {
			return lhs->count != rhs->count ? lhs->count > rhs->count : lhs->val > rhs->val;
		}
	};

	Tree<T> createTree(const std::unordered_map<T, uint32_t>& frequencies) {
		std::priority_queue<typename Tree<T>::node*, std::vector<typename Tree<T>::node*>, nodeCompare> priQ;
		for (auto& p : frequencies) priQ.push(new Tree<T>::node(p.first, p.second));

		while (priQ.size() > 1) {
			auto left = priQ.top();
			priQ.pop();
			auto right = priQ.top();
			priQ.pop();
			auto newNode = new Tree<T>::node(left->val, left->count + right->count, left, right);
			priQ.push(newNode);
		}

		Tree<T> t(priQ.top());
		priQ.pop();

		return t;
	}

 public:
	 compressed compress(const std::string& data) {
		 compressed retVal;

		 if (data.length()) {
			 std::unordered_map<T, uint32_t> frequencies;
			 for (auto c : data) frequencies[c]++;

			 auto tree = createTree(frequencies);
			 auto dictionary = tree.createDictionary();

			 for (auto& f : frequencies)
				 retVal.m_frequencies.push_back(freqPair(f.first, f.second));

			 for (auto ch : data)
				 retVal.m_data.push_back(dictionary[ch]);
		 }

		 return retVal;
	 }

	 std::string decompress(const compressed& cdata) {
		 std::string retVal;

		 if (cdata.m_frequencies.size() > 0 && cdata.m_data.size() > 0) {
			 std::unordered_map<T, uint32_t> frequencies;

			 for (auto& p : cdata.m_frequencies)
				 frequencies[p.val] = p.count;

			 auto tree = createTree(frequencies);

			 typename Tree<T>::node *currNode = tree.Root;

			 for (size_t i = 0; i < cdata.m_data.size(); i++) {
				 bool bit = cdata.m_data[i];
				 currNode = bit ? currNode->right : currNode->left;

				 if (currNode->isLeaf()) {
					 retVal.push_back(currNode->val);
					 currNode = tree.Root;
				 }
			 }
		 }

		 return retVal;
	 }
};