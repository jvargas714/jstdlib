#ifndef JSTDLIB_BIGINT_H
#define JSTDLIB_BIGINT_H
#include <cstdint>
#include <bitset>

/*
 * - 2s complement signed Integer representation of a number up to n bits
 * - operators all defined
 */

template <size_t nbits>
class BigInt {
    std::bitset<nbits> bits;
public:
    BigInt()=default;
    virtual ~BigInt()=default;
    explicit BigInt(int64_t val);
    explicit BigInt(std::string& val);

    BigInt& operator | (const BigInt&);
    BigInt& operator | (uint64_t);

    BigInt& operator & (const BigInt&);
    BigInt& operator & (uint64_t);

    BigInt& operator ^ (const BigInt&);
    BigInt& operator ^ (uint64_t);

    BigInt& operator ++ ();
    BigInt& operator -- ();

    BigInt& operator == (const BigInt&);
    BigInt& operator == (uint64_t);

    BigInt& operator ^= (const BigInt&);
    BigInt& operator ^= (uint64_t);

    BigInt& operator |= (const BigInt&);
    BigInt& operator |= (uint64_t);

    BigInt& operator &= (const BigInt&);
    BigInt& operator &= (uint64_t);
};


#endif //JSTDLIB_BIGINT_H
