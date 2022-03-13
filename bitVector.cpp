#include <stdexcept>

#include "bitVector.h"

bool bitVector::at(size_t index) const {
	if (index > size()) {
		throw std::out_of_range("bitVector::At() : index out of range");
	}
	
	return this->operator[](index);
}

void bitVector::push_back(uint64_t word, uint_fast8_t pushBits) {
	uint_fast8_t way = 2 * (m_freeBits > pushBits) + (m_freeBits < pushBits);

	word &= ((1ULL << pushBits) - 1);

	switch (way) {
	case 0:
		m_data[m_data.size() - 1] |= word;
		m_data.push_back(0);
		m_freeBits = 64;
		break;
	case 1:
		pushBits -= m_freeBits;
		m_data[m_data.size() - 1] |= word >> pushBits;
		word &= (1ULL << pushBits) - 1;
		m_data.push_back(0);
		m_freeBits = 64;
		[[fallthrough]];
	case 2:
		m_freeBits -= pushBits;
		m_data[m_data.size() - 1] |= (word << m_freeBits);
		break;
	}
}

void bitVector::push_back(const bitVector& rhs) {
	for (size_t i = 0; i + 1 < rhs.m_data.size(); i++) {
		this->push_back(rhs.m_data[i], 64);
	}

	this->push_back(rhs.m_data[rhs.m_data.size()-1] >> rhs.m_freeBits, 64-rhs.m_freeBits);
}

void bitVector::pop_back() {
	if (this->empty()) {
		throw std::underflow_error("bitVector::pop_back() : bit vector empty");
	}

	m_freeBits++;

	if (m_freeBits == 65) {
		m_data.pop_back();
		m_freeBits = 1;
	}

	m_data[m_data.size() - 1] &= ~((1ULL << m_freeBits) - 1);
}

size_t bitVector::popcount() const {
	size_t retVal = 0;

	for (auto word : m_data) {
		retVal += std::popcount(word);
	}

	return retVal;
}
