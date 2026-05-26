#include "bigint.hpp"
#define CFLAG(a, b, mask) (((a) & (b) & (mask)) || ((((a) | (b)) & (mask)) & ~(((a) + (b)) & (mask))))
#define MASK 0x8000000000000000ULL
#define MASK2 0xFFFFFFFFFFFFFFFFULL

Integer& Integer::plus_one(const bool ignore_carry) {
    bool carry = true;
    const uint64_t len = array.size();

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = array[i] + carry;
        carry = (array[i] == MASK2) && carry;
        array[i] += sum;
    }

    if (carry && !ignore_carry)
        array.push_back(1);

    return *this;
}

Integer& Integer::plus_(const Integer& other, const bool ignore_carry) {
    bool carry = false;
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const uint64_t len = std::max(this_len, other_len);
    array.resize(len, 0);

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
            sum += array[i]; // carry + array[i]
        } if (i < other_len) {
            carry = CFLAG(sum, other.array[i], MASK);
            sum += other.array[i]; // carry + array[i] + other.array[i]
        }
        array[i] = sum;
    }

    if (carry && !ignore_carry)
        array.push_back(1);

    return *this;
}

Integer& Integer::minus_(const Integer& other) {
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const size_t len = std::max(this_len, other_len) + 1;
    bool carry = false;
    array.resize(len, 0);
    
    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
            sum += array[i]; // carry + array[i]
        } if (i < other_len) {
            carry = CFLAG(sum, ~other.array[i], MASK);
            sum += ~other.array[i]; // carry + array[i] + ~other.array[i]
        } else {
            carry = (sum != 0);
            sum += MASK2;
        }
        array[i] = sum;
    }
    this->plus_one(true);

    if (array.back() & MASK) {
        for (uint64_t i = 0; i < len; i++)
            array[i] = ~array[i];
        this->plus_one(true);
        is_negative = !is_negative;
    }
    if (array.back() == 0)
        array.pop_back();
    return *this;
}

Integer& Integer::_minus(Integer& other) const {
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const size_t len = std::max(this_len, other_len) + 1;
    bool carry = false;
    other.array.resize(len, 0);
    
    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
            sum += array[i]; // carry + array[i]
        } if (i < other_len) {
            carry = CFLAG(sum, ~other.array[i], MASK);
            sum += ~other.array[i]; // carry + array[i] + ~other.array[i]
        } else {
            carry = (sum != 0);
            sum += MASK2;
        }
        other.array[i] = sum;
    }
    other.plus_one(true);

    if (other.array.back() & MASK) {
        for (uint64_t i = 0; i < len; i++)
            other.array[i] = ~array[i];
        other.plus_one(true);
        other.is_negative = !other.is_negative;
    }
    if (other.array.back() == 0)
        other.array.pop_back();
    return other;
}

bool Integer::is_zero() const {
    for (uint64_t num : array)
        if (num != 0)
            return false;
    return true;
}

Integer::Integer() : is_negative(false), array({0}) {}

Integer::Integer(const uint64_t num, const bool b) : is_negative(b), array({num}) {}

Integer::Integer(const std::vector<uint64_t>& arr) : is_negative(false), array(arr) {
    if (array.empty())
        array.push_back(0);
}

Integer::Integer(const Integer& other, const bool b) : is_negative(b), array(other.array) {}

Integer::Integer(const Integer&& other) :is_negative(other.is_negative), array(other.array) {}

Integer& Integer::negate(Integer& n) noexcept {
    n.is_negative = !n.is_negative;
    return n;
}

Integer Integer::negate(const Integer&& n) noexcept {
    return Integer(n, !n.is_negative);
}

Integer Integer::plus(const Integer& other, const bool ignore_carry) const {
    Integer result;
    bool carry = 0;
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const uint64_t len = std::max(this_len, other_len);
    result.array.resize(len, 0);

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
            sum += array[i]; // carry + array[i]
        } if (i < other_len) {
            carry = CFLAG(sum, other.array[i], MASK);
            sum += other.array[i]; // carry + array[i] + other.array[i]
        }
        result.array[i] = sum;
    }

    if (carry && !ignore_carry)
        result.array.push_back(1);

    return result;
}

Integer Integer::minus(const Integer& other) const {
    Integer result(MASK2);
    const uint64_t len = std::max(array.size(), other.array.size()) + 1; // +1은 여유 공간
    result.array.resize(len, MASK2);
    for (uint64_t i = 0; i < other.array.size(); i++) // O(n)의 시간 복잡도로 비트 반전 수행
        result.array[i] = ~other.array[i]; // NOT other.array[i]
    result.plus_one(true).plus_(*this, true); // 1을 더하여 음수로 변환 후 덧셈 연산
    if (result.array.back() & MASK) { // 음수라면 이를 다시 양수로 바꾸기 위해
        for (uint64_t i = 0; i < len; i++) // O(n)의 시간 복잡도로 비트 반전 수행
            result.array[i] = ~result.array[i]; // NOT result.array[i]
        result.plus_one(true); // 1을 더하여 음수로 변환
        result.is_negative = true; // 음수임을 표시
    }
    if (result.array.back() == 0) // 최상위 8바이트가 0이면 제거하여 공간 절약
        result.array.pop_back();
    return result;
}

Integer Integer::operator-() const {
    return Integer(*this, !is_negative);
}

Integer Integer::operator~() const {
    Integer result;
    result.array.resize(array.size(), 0);
    for (size_t i = 0; i < array.size(); i++)
        result.array[i] = ~array[i];
    return result;
}

Integer Integer::operator<<(const u_int64_t c) const {
    const u_int64_t r = c >> 6; // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    // c = 64r + q
    Integer result;
    result.array.resize(array.size() + r, 0);
    for (int64_t i = r; i < result.array.size(); i++) // this << r
        result.array[i] = array[i - r];
    if (q == 0)
        return result;
    // 63 >= q >= 1 <=> 62 >= q - 1 >= 0
    const u_int64_t mask = (~(MASK >> (q - 1)) + 1); // 비트 마스크
    const u_int32_t shift = 64 - q;
    if (result.array.back() & mask) // 시프트 연산으로 인해 범위를 벗어나는 비트가 있는 경우
        result.array.push_back(0);
    for (int64_t i = result.array.size() - 1; i > 0; i--) {
        result.array[i] <<= q;
        result.array[i] |= (array[i - 1] & mask) >> shift; // 아래 주석과 결과가 동일함
        // result.array[i] += (array[i - 1] & mask) >> shift;
    }

    result.array[0] <<= q;
    return result;
}

Integer Integer::operator+(const Integer& other) const {
    if (is_negative == other.is_negative)
        return Integer(this->plus(other), is_negative); // (-this) + (-other) = -(this + other), this + other
    else if (is_negative) // other - this
        return other.minus(*this);
    else // this - other
        return this->minus(other);
}

Integer& Integer::operator+=(const Integer& other) {
    if (is_negative == other.is_negative)
        return this->plus_(other);
    else if (is_negative)
        return other._minus(*this);
    else
        return this->minus_(other);
}

Integer Integer::operator-(const Integer& other) const {
    if (is_negative == other.is_negative && is_negative) { // (-this) - (-other) = (-this) + other = -(this - other)
        return Integer::negate(this->minus(other));
    }
    else if (is_negative == other.is_negative && !is_negative) // this - other
        return this->minus(other);
    else if (is_negative) // (-this) - other = (-this) + (-other) = -(this + other)
        return Integer(this->plus(other), true);
    else // this - (-other) = this + other
        return this->plus(other);
}

Integer& Integer::operator-=(const Integer& other) {
    if (is_negative == other.is_negative && is_negative) // (-this) - (-other) = (-this) + other = -(this - other)
        return Integer::negate(this->minus_(other));
    else if (is_negative == other.is_negative && !is_negative) // this - other
        return this->minus_(other);
    else if (is_negative) // (-this) - other = (-this) + (-other) = -(this + other)
        return this->plus_(other);
    else // this - (-other) = this + other
        return this->plus_(other);
}

bool Integer::operator<=(const Integer& other) const {
    if (is_zero() && other.is_zero())
        return true;
    if (is_negative != other.is_negative)
        return is_negative ? true : false;
    for (size_t i = array.size(); i < std::max(array.size(), other.array.size()); i++)
        if (other.array[i] != 0)
            return true;
    for (size_t i = other.array.size(); i < std::max(array.size(), other.array.size()); i++)
        if (array[i] != 0)
            return false;
    for (size_t i = std::min(array.size(), other.array.size()); i > 0; i--)
        if (array[i - 1] > other.array[i - 1])
            return false;
    return true;
}

bool Integer::operator!=(const Integer& other) const {
    if (is_zero && other.is_zero())
        return false;
}
