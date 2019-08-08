#include "BigInt.h"

/*
 * note: bitset -- right most (lsb) is index 0
 */

// fills bitset from int, returns true if a negative value
template<size_t nbits>
static bool fillBitsSet(int64_t val, std::bitset<nbits>& bs) {
    bs.set(val);
    return val < 0;
}

template<size_t nbits>
BigInt<nbits>::BigInt(int64_t val) {
    m_neg = fillBitsSet(val, m_bits);
}

template<size_t nbits>
BigInt<nbits>::BigInt(std::string &val) {

}

template<size_t nbits>
BigInt<nbits>::BigInt(const BigInt &) {

}

template<size_t nbits>
BigInt<nbits>::BigInt(BigInt &&) {

}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator|(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator|(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator&(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator&(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator^(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator^(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator++() {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator--() {
    return <#initializer#>;
}

template<size_t nbits>
BigInt &BigInt<nbits>::operator==(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator==(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator^=(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator^=(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator|=(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator|=(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator&=(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator&=(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator~() {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator=(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator=(BigInt &&) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> BigInt<nbits>::operator++(int) {
    return BigInt();
}

template<size_t nbits>
BigInt<nbits> BigInt<nbits>::operator--(int) {
    return BigInt();
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator!=(const BigInt &) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> &BigInt<nbits>::operator!=(int64_t) {
    return <#initializer#>;
}

template<size_t nbits>
BigInt<nbits> BigInt<nbits>::operator+(const BigInt &) {
    return BigInt();
}

template<size_t nbits>
BigInt<nbits> BigInt<nbits>::operator-(const BigInt &) {
    return BigInt();
}

template<size_t nbits>
BigInt<nbits> BigInt<nbits>::operator*(const BigInt &) {
    return BigInt();
}

template<size_t nbits>
BigInt<nbits> BigInt<nbits>::operator/(const BigInt &) {
    return BigInt();
}

