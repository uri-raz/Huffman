#pragma once

#include <vector>

class bitVector
{
public:
	std::vector<uint64_t> m_data;
	uint_fast8_t m_freeBits;

	bitVector() : m_freeBits(64)  {
		m_data.push_back(0);
	}

	bool empty() const noexcept {
		return m_data.size() == 1 && m_freeBits == 64;
	}

	size_t size() const noexcept {
		return m_data.size() * 64 - m_freeBits;
	}

	void resize(size_t numBits) {
		size_t numWords = numBits / 64 + 1;
		m_freeBits = uint_fast8_t(64 - (numBits - 64 * (numWords - 1)));
		m_data.resize(numWords);
	}

	void clear() noexcept {
		m_data.clear();
		m_data.push_back(0);
		m_freeBits = 64;
	}

	void swap(bitVector &other) {
		std::swap(m_data, other.m_data);
		std::swap(m_freeBits, other.m_freeBits);
	}

	bool operator[](size_t index) const {
		size_t word = index / 64;
		size_t bit = 63 - (index - 64 * word);

		return (m_data[word] & (1ULL << bit)) >> bit;
	}

	bool at(size_t index) const;
	void push_back(uint64_t word, uint_fast8_t pushBits = 1);
	void push_back(const bitVector& rhs);
	void pop_back();
	size_t popcount() const;
};