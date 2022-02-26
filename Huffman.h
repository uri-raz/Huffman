#pragma once

#include <map>
#include <queue>
#include <vector>
#include <cstddef>
#include <fstream>
#include <unordered_map>

class Huffman
{
 public:
	struct freqPair {
		char	 val;
		uint32_t count;
	};

	class compressed {
	public:
		std::vector<freqPair> frequencies;
		std::vector<bool> data;

		bool isFull()  { return data.size() >  0; }
		bool isEmpty() { return data.size() == 0; }

		void save(const std::string& fileName) {
			std::ofstream outfile(fileName, std::ofstream::binary);

			uint64_t word = frequencies.size();
			outfile.write((const char*)&word, sizeof(uint64_t));

			for (auto p : frequencies) {
				outfile.write(&p.val, sizeof(char));
				outfile.write((const char*)&p.count, sizeof(uint32_t));
			}

			word = data.size();
			outfile.write((const char*)&word, sizeof(uint64_t));

			uint_fast8_t bits = 0;
			for (size_t i = 0; i < data.size(); i++) {
				word = word << 1 | data[i];
				bits++;

				if (bits == sizeof(uint64_t) * CHAR_BIT) {
					outfile.write((const char*)&word, sizeof(uint64_t));
					bits = 0;
				}
			}

			if (bits != 0) {
				word <<= (sizeof(uint64_t) * CHAR_BIT - bits);
				outfile.write((const char*)&word, sizeof(uint64_t));
			}

			outfile.close();
		}

		void load(const std::string& fileName) {
			data.clear();
			frequencies.clear();

			std::ifstream infile(fileName, std::ofstream::binary);
			uint64_t word;
			infile.read((char*)&word, sizeof(uint64_t));

			freqPair p;
			for (uint64_t i = 0; i < word; i++) {
				infile.read(&p.val, sizeof(char));
				infile.read((char*)&p.count, sizeof(uint32_t));
				frequencies.push_back(p);
			}

			uint64_t totalBits, mask = 0;
			infile.read((char*)&totalBits, sizeof(uint64_t));

			for (size_t i = 0; i < totalBits; i++) {
				if (mask == 0) {
					infile.read((char*)&word, sizeof(uint64_t));
					mask = 1ULL << 63;
				}

				data.push_back((word & mask) != 0);
				mask >>= 1;
			}

			infile.close();
		}
	};

 private:
	class Tree
	{
	public:
		enum class status : std::uint8_t { Success, Duplicate, NotFound };

		struct node
		{
			node(char v = '\0', uint32_t c = 0, node* l = nullptr, node* r = nullptr) : val(v), count(c), left(l), right(r) {}

			char val;
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

		std::map<char, std::vector<bool>> createDictionary() {
			std::vector<bool> prefix;
			std::map<char, std::vector<bool>> dictionary;

			if (Root)
				createDictionary(Root, prefix, dictionary);

			return dictionary;
		}

	private:
		void createDictionary(const node* Node,
							  std::vector<bool>& currPrefix,
							  std::map<char, std::vector<bool>>& dictionary) {
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
		bool operator()(const Tree::node* lhs, const Tree::node* rhs) {
			if (lhs->count != rhs->count)
				return lhs->count > rhs->count;
			else
				return lhs->val > rhs->val;
		}
	};

	Tree createTree(const std::unordered_map<char, uint32_t>& frequencies) {
		std::priority_queue<Tree::node*, std::vector<Tree::node*>, nodeCompare> priQ;
		for (auto& p : frequencies) priQ.push(new Tree::node(p.first, p.second));

		while (priQ.size() > 1) {
			auto left = priQ.top();
			priQ.pop();
			auto right = priQ.top();
			priQ.pop();
			auto newNode = new Tree::node(left->val, left->count + right->count, left, right);
			priQ.push(newNode);
		}

		Tree t(priQ.top());
		priQ.pop();

		return t;
	}

 public:
	 compressed compress(const std::string& data) {
		 compressed retVal;

		 if (data.length()) {
			 std::unordered_map<char, uint32_t> frequencies;
			 for (auto c : data) frequencies[c]++;

			 auto tree = createTree(frequencies);
			 auto dictionary = tree.createDictionary();

			 for (auto& f : frequencies)
				 retVal.frequencies.push_back(freqPair(f.first, f.second));

			 for (auto ch : data)
				 std::copy(dictionary[ch].begin(), dictionary[ch].end(), std::back_inserter(retVal.data));
		 }

		 return retVal;
	 }

	 std::string decompress(const compressed& cdata) {
		 std::string retVal;

		 if (cdata.frequencies.size() > 0 && cdata.data.size() > 0) {
			 std::unordered_map<char, uint32_t> frequencies;

			 for (auto& p : cdata.frequencies)
				 frequencies[p.val] = p.count;

			 auto tree = createTree(frequencies);

			 Tree::node *currNode = tree.Root;

			 for (auto bit : cdata.data) {
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