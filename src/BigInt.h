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
    std::bitset<nbits> m_bits;
    bool m_neg;
public:
    BigInt()=default;
    virtual ~BigInt()=default;
    explicit BigInt(int64_t val);
    explicit BigInt(std::string& val);
    BigInt(const BigInt&) noexcept;
    BigInt(BigInt&&) noexcept;

    // assign
    BigInt& operator = (const BigInt&) noexcept;
    BigInt& operator = (BigInt&&)noexcept;

    // bitwise
    BigInt& operator | (const BigInt&);
    BigInt& operator | (int64_t);

    BigInt& operator & (const BigInt&);
    BigInt& operator & (int64_t);

    BigInt& operator ^ (const BigInt&);
    BigInt& operator ^ (int64_t);

    BigInt& operator ^= (const BigInt&);
    BigInt& operator ^= (int64_t);

    BigInt& operator |= (const BigInt&);
    BigInt& operator |= (int64_t);

    BigInt& operator &= (const BigInt&);
    BigInt& operator &= (int64_t);

    BigInt& operator ~ ();

    // increment / decrement
    BigInt& operator ++ ();     // ++bs
    BigInt& operator -- ();     // --bs
    BigInt operator ++ (int);  // bs++
    BigInt operator -- (int);  // bs--< int is a dummy arg to distinguish between pre decr and post decr (same for ++)

    // compare
    BigInt& operator == (const BigInt&);
    BigInt& operator == (int64_t);

    BigInt& operator != (const BigInt&);
    BigInt& operator != (int64_t);

    // arithmetic
    BigInt operator + (const BigInt&);
    BigInt operator - (const BigInt&);
    BigInt operator * (const BigInt&);
    BigInt operator / (const BigInt&);
};


#endif //JSTDLIB_BIGINT_H
