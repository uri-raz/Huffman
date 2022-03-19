#pragma once

#include <iterator>
#include <vector>

class bitVector
{
public:
	std::vector<uint64_t> m_data;
	uint_fast8_t m_freeBits;

	/*****************************************************************************/
	/* Proxy to allow non-const operator[] to return a reference to a bit		 */
	class bitProxy
	{
		friend class bitVector;

	private:
		size_t bitNumber;
		bitVector& myVector;

		bitProxy(size_t index, bitVector& vec) : bitNumber(index), myVector(vec) {}

	public:
		bitProxy& operator=(bool val);

		operator bool() const {
			return myVector.at(bitNumber);
		}
	};
	/*																			 */
	/*****************************************************************************/

	/************************************************************************************/
	/* iterator implementation															*/
	class iterator
	{
		friend class bitVector;

	public:
		using value_type = bool;
		using pointer	 = bitProxy;
		using reference	 = bitProxy;
		using iterator_category	= std::contiguous_iterator_tag;

		iterator() : proxy(nullptr) {}

		iterator operator++(int) {
			proxy->bitNumber = std::min(proxy->bitNumber + 1, proxy->myVector.size() + 1);

			return *this;
		}

		iterator operator++() {
			auto tmp = this;
			proxy->bitNumber = std::min(proxy->bitNumber + 1, proxy->myVector.size() + 1);

			return *tmp;
		}

		iterator operator--(int) {
			auto tmp = this;
			proxy->bitNumber = proxy->bitNumber ? proxy->bitNumber-- : 0;

			return *tmp;
		}

		iterator operator--() {
			proxy->bitNumber = proxy->bitNumber ? proxy->bitNumber-- : 0;

			return *this;
		}

		iterator operator=(bool rhs) {
			*proxy = rhs;
			return *this;
		}

		bool operator==(const iterator& rhs) const {
			return &proxy->myVector == &rhs.proxy->myVector && proxy->bitNumber == rhs.proxy->bitNumber;
		}

		bool operator<(const iterator& rhs) const {
			return &proxy->myVector == &rhs.proxy->myVector && proxy->bitNumber < rhs.proxy->bitNumber;
		}

		bool operator>(const iterator& rhs) const {
			return &proxy->myVector == &rhs.proxy->myVector && proxy->bitNumber > rhs.proxy->bitNumber;
		}

		bool operator*() {
			bool retVal = *proxy;
			return retVal;
		}

	private:
		bitVector::bitProxy *proxy;
	};
	/*																					*/
	/************************************************************************************/

	bitVector() : m_freeBits(64) {
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

	void swap(bitVector& other) {
		std::swap(m_data, other.m_data);
		std::swap(m_freeBits, other.m_freeBits);
	}

	bool operator[](size_t index) const {
		size_t word = index / 64;
		size_t bit = 63 - (index - 64 * word);

		return (m_data[word] & (1ULL << bit)) >> bit;
	}

	bitProxy operator[](size_t index) {
		return bitProxy(index, *this);
	}

	bool at(size_t index) const;
	void push_back(uint64_t word, uint_fast8_t pushBits = 1);
	void push_back(bool val) { push_back(uint64_t(val), 1); }
	void push_back(const bitVector& rhs);
	void pop_back();
	void clear() noexcept;
	size_t popcount() const;

	iterator begin() {
		iterator retVal;
		retVal.proxy = new bitProxy(0ULL, *this);

		return retVal;
	}

	iterator end() {
		iterator retVal;
		retVal.proxy = new bitProxy(size(), *this);

		return retVal;
	}
};