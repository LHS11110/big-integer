#include "bigint.hpp"
#include <stdexcept>
#define CFLAG(a, b, mask)                                                      \
    (((a) & (b) & (mask)) || ((((a) | (b)) & (mask)) & ~(((a) + (b)) & (mask))))
#define MASK 0x8000000000000000ULL
#define MASK2 0xFFFFFFFFFFFFFFFFULL

bool is_number(const std::string& str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (i == 0 && (str[i] == 45 || str[i] == 43)) // 첫 문자가 +, -인 경우
            continue;
        if ((48 > str[i] || str[i] > 57) && str[i] != 44) // 쉼표가 아니면서 숫자도 아닌 경우
            return false;
    }
    return true;
}

uint8_t reverse_8bit(uint8_t n) {
    n = ((n & 0xF0) >> 4) | ((n & 0x0F) << 4);
    n = ((n & 0xCC) >> 2) | ((n & 0x33) << 2);
    n = ((n & 0xAA) >> 1) | ((n & 0x55) << 1);
    return n;
}

uint16_t reverse_16bit(uint16_t n) {
    return (static_cast<uint16_t>(reverse_8bit(n)) << 8) | reverse_8bit(n >> 8);
}

uint32_t reverse_32bit(uint32_t n) {
    return (static_cast<uint32_t>(reverse_16bit(n)) << 16) | reverse_16bit(n >> 16);
}

uint64_t reverse_64bit(uint64_t n) {
    return (static_cast<uint64_t>(reverse_32bit(n)) << 32) | reverse_32bit(n >> 32);
}

uint64_t _log2(const uint64_t n) {
    uint64_t m = n;
    uint64_t result = 0;
    if (m & 0xFFFFFFFF00000000ULL) {
        result += 32;
        m >>= 32;
    } if (m & 0xFFFF0000ULL) {
        result += 16;
        m >>= 16;
    } if (m & 0xFF00ULL) {
        result += 8;
        m >>= 8;
    } if (m & 0xF0ULL) {
        result += 4;
        m >>= 4;
    } if (m & 0xCULL) {
        result += 2;
        m >>= 2;
    } if (m & 0x2ULL) {
        result += 1;
    }
    return result;
}

Integer &Integer::plus_one(const bool ignore_carry) {
    bool carry = true;
    const uint64_t len = array.size();

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = array[i] + carry;
        carry = (array[i] == MASK2) && carry;
        array[i] = sum;
    }

    if (carry && !ignore_carry)
        array.push_back(1);

    return *this;
}

Integer &Integer::plus_(const Integer &other, const bool ignore_carry) {
    bool carry = false;
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const uint64_t len = std::max(this_len, other_len);
    array.resize(len, 0);

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        carry = false;
        if (i < this_len) {
            carry = array[i] == MASK2 && sum == 1;
            sum += array[i]; // carry + array[i]
        }
        if (i < other_len) {
            carry = CFLAG(sum, other.array[i], MASK) || carry;
            sum += other.array[i]; // carry + array[i] + other.array[i]
        }
        array[i] = sum;
    }

    if (carry && !ignore_carry)
        array.push_back(1);

    return *this;
}

Integer &Integer::minus_(const Integer &other) {
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const size_t len = std::max(this_len, other_len) + 1;
    bool carry = false;
    array.resize(len, 0);

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        carry = false;
        if (i < this_len) {
            carry = array[i] == MASK2 && sum == 1;
            sum += array[i]; // carry + array[i]
        }
        if (i < other_len) {
            carry = CFLAG(sum, ~other.array[i], MASK) || carry;
            sum += ~other.array[i]; // carry + array[i] + ~other.array[i]
        } else {
            carry = (sum != 0) || carry;
            sum += MASK2;
        }
        array[i] = sum;
    }
    this->plus_one(true);

    if (array.back() & MASK) { // 결과가 음수라면
        for (uint64_t i = 0; i < len; i++)
        array[i] = ~array[i];
        this->plus_one(true);
        is_negative = true;
    } else
        is_negative = false;
    normalize();
    return *this;
}

Integer &Integer::_minus(Integer &other) const {
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const size_t len = std::max(this_len, other_len) + 1;
    bool carry = false;
    other.array.resize(len, 0);

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        carry = false;
        if (i < this_len) {
            carry = array[i] == MASK2 && sum == 1;
            sum += array[i]; // carry + array[i]
        }
        if (i < other_len) {
            carry = CFLAG(sum, ~other.array[i], MASK) || carry;
            sum += ~other.array[i]; // carry + array[i] + ~other.array[i]
        } else {
            carry = (sum != 0) || carry;
            sum += MASK2;
        }
        other.array[i] = sum;
    }
    other.plus_one(true);

    if (other.array.back() & MASK) { // 결과가 음수라면
        for (uint64_t i = 0; i < len; i++)
        other.array[i] = ~other.array[i];
        other.plus_one(true);
        other.is_negative = true;
    } else // 결과가 음이 아닌 정수라면
        other.is_negative = false;
    other.normalize();
    return other;
}

bool Integer::is_zero() const {
    for (uint64_t num : array)
        if (num != 0)
            return false;
    return true;
}

Integer Integer::mod(const Integer& n, const uint64_t l) {
    const uint64_t q = l >> 6;
    const uint64_t r = l & 0x3FULL;
    Integer result;
    result.array.resize(q + 1, 0); // result.array의 크기는 l에 의해 결정됨
    for (size_t i = 0; i < std::min(q + 1, static_cast<uint64_t>(n.array.size())); i++) // O(l)
        result.array[i] = n.array[i];
    const uint64_t mask = (1ULL << r) + ~(0ULL);
    result.array[q] &= mask;
    result.normalize(); // O(l)
    if (n.is_negative) {
        Integer modulo(1);
        modulo <<= l; // 2^l, O(l)
        return std::move(modulo - result); // O(l)
    }
    return std::move(result);
}

Integer Integer::operator/(const Integer& other) const {
    if (other.is_zero()) // O(m)
        throw std::invalid_argument("Division by zero");
    Integer result = std::move(Integer::abs(*this)); // O(n)
    Integer d = std::move(Integer::abs(other)); // O(m)
    if (result.is_zero()) // O(n)
        return 0;
    if (result < d) { // O(max(n, m))
        if (this->is_negative ^ other.is_negative)
            return std::move(Integer(1, true));
        return 0;
    }
    if (result == d) { // O(max(n, m))
        if (this->is_negative ^ other.is_negative)
            return std::move(Integer(1, true));
        return 1;
    }
    // |other| < |this|
    const int64_t n = Integer::LSB(d); // O(m) < O(n)
    result >>= n; // O(n)
    d >>= n; // O(m) < O(n)
    const Integer r = std::move(mod_odd(result, d)); // O(n^log_2 3 log_2 n)
    result -= r; // O(n)
    const uint64_t deg_result = Integer::log2(result); // O(n)
    Integer inverse_d = Integer::inverse_of(d, deg_result + 1); // O(M(n))
    result = std::move(Integer::mod(result * inverse_d, deg_result + 1)); // O(M(n))
    result.is_negative = this->is_negative ^ other.is_negative;
    if (result.is_negative && !r.is_zero()) // O(m) < O(n)
        result.plus_one(true); // O(n)
    return std::move(result);
}

void Integer::normalize() {
    size_t len = 1;
    for (size_t i = 1; i < array.size(); i++)
        if (array[i] != 0)
            len = i + 1;
    array.resize(len);
}

int64_t Integer::log2(const Integer& n) {
    for (size_t i = n.array.size(); i > 0; i--)
        if (n.array[i - 1] != 0)
            return ((i - 1) << 6) + _log2(n.array[i - 1]);
    return -1; // log 0은 정의되지 않지만, ⌊log n⌋ + 1은 보통 비트의 길이를 나타낸다. 따라서 -1로 정의하는 것이 자연스럽다.
}

int64_t Integer::MSB(const Integer& n) {
    return Integer::log2(n);
}

int64_t Integer::LSB(const Integer& n) {
    int64_t result = -1;
    for (size_t i = n.array.size(); i > 0; i--)
        if (n.array[i - 1] != 0)
            result = ((i - 1) << 6) + (_log2(n.array[i - 1] & (~n.array[i - 1] + 1)));
    return result;
}

Integer Integer::karatsuba_mul(const Integer &other) const {
    if (is_zero() || other.is_zero())
        return std::move(Integer(0));
    if (array.size() == 1 && other.array.size() == 1) {
        static const uint64_t mask = 0xFFFFFFFF00000000ULL;
        uint64_t high = (array[0] & mask) >> 32;
        uint64_t low = array[0] & ~mask;
        uint64_t other_high = (other.array[0] & mask) >> 32;
        uint64_t other_low = other.array[0] & ~mask;

        // a = high * 2^32 + low
        // b = other_high * 2^32 + other_low
        // ab = high * other_high * 2^64 + (high * other_low + low * other_high) * 2^32 + low * other_low
        uint64_t z0 = low * other_low;
        uint64_t z1_l = high * other_low;
        uint64_t z1_r = low * other_high;
        uint64_t z2 = high * other_high;
        Integer result(z2);
        result <<= 32;
        result += z1_l;
        result += z1_r;
        result <<= 32;
        result += z0;
        result.is_negative = is_negative ^ other.is_negative;
        return std::move(result);
    }
    // 1 < array.size() || 1 < other.array.size()
    const uint64_t len = std::max(array.size(), other.array.size());
    const uint64_t log2_len = _log2(len);
    const uint64_t len2 = log2_len + (((1ULL << log2_len) - 1) & len ? 1 : 0); // 2의 제곱수로 맞춘 길이
    // 1 < 2^(len2 - 1) < len <= 2^len2
    // 1 <= len2
    Integer x0;
    Integer x1;
    const uint64_t half_len = 1 << (len2 - 1);
    if (array.size() <= half_len) {
        x0 = *this;
        x0.is_negative = false; // x0, x1, y0, y1은 항상 양의 정수
    } else { // half_len < array.size()
        x1.array.resize(array.size() - half_len, 0);
        for (size_t i = 0; i < array.size() - half_len; i++)
            x1.array[i] = array[i + half_len];
        x0.array.resize(half_len, 0);
        for (size_t i = 0; i < half_len; i++)
            x0.array[i] = array[i];
    }
    Integer y0;
    Integer y1;
    if (other.array.size() <= half_len) {
        y0 = other;
        y0.is_negative = false;
    } else {
        y1.array.resize(other.array.size() - half_len, 0);
        for (size_t i = 0; i < other.array.size() - half_len; i++)
            y1.array[i] = other.array[i + half_len];
        y0.array.resize(half_len, 0);
        for (size_t i = 0; i < half_len; i++)
            y0.array[i] = other.array[i];
    }
    Integer z0 = std::move(x0.karatsuba_mul(y0));
    Integer z2 = std::move(x1.karatsuba_mul(y1));
    Integer z3 = std::move(x1.plus_(x0).karatsuba_mul(y1.plus_(y0)));
    Integer z1 = std::move(z3.minus_(z2 + z0));
    return (is_negative ^ other.is_negative) ? std::move(negate(std::move(z0 + (z1 << (half_len << 6)) + (z2 << (half_len << 7)))))
                                        : (std::move(z0 + (z1 << (half_len << 6)) + (z2 << (half_len << 7))));
}

Integer& Integer::abs(Integer& n) {
    n.is_negative = false;
    return n;
}

Integer Integer::abs(Integer&& n) {
    n.is_negative = false;
    return std::move(n);
}

Integer Integer::abs(const Integer& n) {
    Integer result = n;
    result.is_negative = false;
    return std::move(result);
}

Integer::Integer() : is_negative(false), array({0}) {}

Integer::Integer(const uint64_t num, const bool b)
    : is_negative(b), array({num}) {}

Integer::Integer(const std::vector<uint64_t> &arr)
    : is_negative(false), array(arr) {
    if (array.empty())
        array.push_back(0);
}

Integer::Integer(const Integer &other, const bool b)
    : is_negative(b), array(other.array) {}

Integer::Integer(Integer &&other)
    : is_negative(other.is_negative), array(std::move(other.array)) {}

Integer::Integer(Integer &&other, const bool b)
    : is_negative(b), array(std::move(other.array)) {}

Integer::Integer(const std::string& str) : Integer() {
    if (str.empty())
        return;
    if (!is_number(str))
        throw std::invalid_argument("The argument must be a valid numeric string (e.g., '123' or '-123')");
    for (size_t i = 0; i < str.size(); i++) {
        if (str[i] == 45)
            is_negative = true;
        if (48 <= str[i] && str[i] <= 57) {
            *this = std::move((*this << 3).plus_(*this << 1));
            // this = 10 * this;
            this->plus_(str[i] - 48);
        }
    }
}

Integer::Integer(const std::vector<uint64_t>::iterator _first, const std::vector<uint64_t>::iterator _last, const uint64_t mask, const bool b) : array(_first, _last), is_negative(b) {
    if (array.empty())
        array.push_back(0);
    else
        array.back() &= mask;
}

Integer &Integer::negate(Integer &n) noexcept {
    n.is_negative = !n.is_negative;
    return n;
}

Integer Integer::negate(Integer &&n) noexcept {
    return Integer(std::move(n), !n.is_negative);
}

Integer Integer::plus(const Integer &other, const bool ignore_carry) const {
    Integer result;
    bool carry = 0;
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const uint64_t len = std::max(this_len, other_len);
    result.array.resize(len, 0);

    uint64_t sum = 0;
    for (uint64_t i = 0; i < len; i++) {
        sum = carry; // carry
        carry = false;
        if (i < this_len) {
        carry = array[i] == MASK2 && sum == 1;
        sum += array[i]; // carry + array[i]
        }
        if (i < other_len) {
        carry = CFLAG(sum, other.array[i], MASK) || carry;
        sum += other.array[i]; // carry + array[i] + other.array[i]
        }
        result.array[i] = sum;
    }

    if (carry && !ignore_carry)
        result.array.push_back(1);

    return std::move(result);
}

Integer Integer::minus(const Integer &other) const {
    Integer result(MASK2);
    const uint64_t len =
        std::max(array.size(), other.array.size()) + 1; // +1은 여유 공간
    result.array.resize(len, MASK2);
    for (uint64_t i = 0; i < other.array.size();
        i++)                            // O(n)의 시간 복잡도로 비트 반전 수행
        result.array[i] = ~other.array[i]; // NOT other.array[i]
    result.plus_one(true).plus_(*this,
                                true);   // 1을 더하여 음수로 변환 후 덧셈 연산
    if (result.array.back() & MASK) {    // 음수라면 이를 다시 양수로 바꾸기 위해
        for (uint64_t i = 0; i < len; i++) // O(n)의 시간 복잡도로 비트 반전 수행
            result.array[i] = ~result.array[i]; // NOT result.array[i]
        result.plus_one(true);                // 1을 더하여 음수로 변환
        result.is_negative = true;            // 음수임을 표시
    }
    result.normalize();
    return std::move(result);
}

Integer Integer::operator-() const { return std::move(Integer(*this, !is_negative)); }

Integer Integer::operator~() const {
    Integer result;
    result.array.resize(array.size(), 0);
    for (size_t i = 0; i < array.size(); i++)
        result.array[i] = ~array[i];
    return std::move(result);
}

Integer Integer::operator<<(const uint64_t c) const {
    if (c == 0)
        return *this;
    const uint64_t r = c >> 6;     // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    // c = 64r + q
    Integer result;
    result.is_negative = is_negative;
    result.array.resize(array.size() + r, 0);
    for (int64_t i = r; i < result.array.size(); i++) // this << r
        result.array[i] = array[i - r];
    if (q == 0)
        return std::move(result);
    // 63 >= q >= 1 <=> 62 >= q - 1 >= 0
    const uint64_t mask = (~(MASK >> (q - 1)) + 1); // 비트 마스크
    const uint32_t shift = 64 - q;
    if (result.array.back() &
        mask) // 시프트 연산으로 인해 범위를 벗어나는 비트가 있는 경우
        result.array.push_back(0);
    for (int64_t i = result.array.size() - 1; i > r; i--) {
        result.array[i] <<= q;
        result.array[i] |=
            result.array[i - 1] >> shift; // 아래 주석과 결과가 동일함
        // result.array[i] += (result.array[i - 1] & mask) >> shift;
    }
    result.array[r] <<= q;
    return std::move(result);
}

Integer Integer::operator>>(const uint64_t c) const {
    const uint64_t r = c >> 6;     // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    if (array.size() <= r)
        return std::move(Integer());
    Integer result(0, is_negative);
    result.array.resize(array.size() - r, 0);
    for (size_t i = 0; i < result.array.size(); i++)
        result.array[i] = array[i + r];
    if (q == 0)
        return std::move(result);
    // const uint64_t mask = MASK2 + (1 << q);
    const uint32_t shift = 64 - q;
    for (size_t i = 0; i < result.array.size() - 1; i++) {
        result.array[i] >>= q;
        result.array[i] |= result.array[i + 1] << shift;
    }
    result.array.back() >>= q;
    result.normalize();
    return std::move(result);
}

Integer& Integer::operator<<=(const uint64_t c) {
    const uint64_t r = c >> 6;     // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    array.resize(array.size() + r, 0);
    for (size_t i = array.size(); i > r; i--)
        array[i - 1] = array[i - r - 1];
    for (size_t i = 0; i < r; i++)
        array[i] = 0;
    if (q == 0)
        return *this;
    const uint64_t mask = (~(MASK >> (q - 1)) + 1); // 비트 마스크
    const uint32_t shift = 64 - q;
    if (array.back() & mask) // 시프트 연산으로 인해 범위를 벗어나는 비트가 있는 경우
        array.push_back(0);
    for (int64_t i = array.size() - 1; i > r; i--) {
        array[i] <<= q;
        array[i] |=
            (array[i - 1] & mask) >> shift;
    }
    array[r] <<= q;
    return *this;
}

Integer& Integer::operator>>=(const uint64_t c) {
    const uint64_t r = c >> 6;     // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    if (array.size() <= r) {
        array.clear();
        array.push_back(0);
        is_negative = false;
        return *this;
    }
    for (size_t i = 0; i < array.size() - r; i++)
        array[i] = array[i + r];
    array.resize(array.size() - r, 0);
    if (q == 0)
        return *this;
    // const uint64_t mask = MASK2 + (1 << q);
    const uint32_t shift = 64 - q;
    for (size_t i = 0; i < array.size() - 1; i++) {
        array[i] >>= q;
        array[i] |= array[i + 1] << shift;
    }
    array.back() >>= q;
    normalize();
    return *this;
}

Integer Integer::operator+(const Integer &other) const {
    if (is_negative == other.is_negative)
        return Integer(
            this->plus(other),
            is_negative);   // (-this) + (-other) = -(this + other), this + other
    else if (is_negative) // other - this
        return other.minus(*this);
    else // this - other
        return this->minus(other);
}

Integer &Integer::operator+=(const Integer &other) {
    if (is_negative == other.is_negative)
        return this->plus_(other);
    else if (is_negative)
        return other._minus(*this);
    else
        return this->minus_(other);
}

Integer Integer::operator-(const Integer &other) const {
    if (is_negative == other.is_negative &&
        is_negative) // (-this) - (-other) = (-this) + other = -(this - other)
        return Integer::negate(this->minus(other));
    else if (is_negative == other.is_negative && !is_negative) // this - other
        return this->minus(other);
    else if (is_negative) // (-this) - other = (-this) + (-other) = -(this +
                            // other)
        return Integer(this->plus(other), true);
    else // this - (-other) = this + other
        return this->plus(other);
}

Integer &Integer::operator-=(const Integer &other) {
    if (is_negative == other.is_negative &&
        is_negative) // (-this) - (-other) = (-this) + other = -(this - other)
        return Integer::negate(this->minus_(other));
    else if (is_negative == other.is_negative && !is_negative) // this - other
        return this->minus_(other);
    else if (is_negative) // (-this) - other = (-this) + (-other) = -(this +
                            // other)
        return this->plus_(other);
    else // this - (-other) = this + other
        return this->plus_(other);
}

bool Integer::operator<=(const Integer &other) const {
    if (is_zero() && other.is_zero())
        return true;
    if (is_negative != other.is_negative)
        return is_negative ? true : false;

    for (size_t i = array.size(); i < std::max(array.size(), other.array.size());
        i++)
        if (other.array[i] != 0) // |this| < |other|
            return is_negative ? false : true;
    for (size_t i = other.array.size();
        i < std::max(array.size(), other.array.size()); i++)
        if (array[i] != 0) // |this| > |other|
            return is_negative ? true : false;
    for (size_t i = std::min(array.size(), other.array.size()); i > 0; i--) {
        if (array[i - 1] > other.array[i - 1]) // |this| > |other|
            return is_negative ? true : false;
        else if (array[i - 1] < other.array[i - 1]) // |this| < |other|
            return is_negative ? false : true;
    }
    return true; // |this| = |other|
}

bool Integer::operator<(const Integer& other) const {
    return this->operator<=(other) && this->operator!=(other);
}

bool Integer::operator!=(const Integer &other) const {
    if (is_zero() && other.is_zero())
        return false;
    if (is_negative != other.is_negative)
        return true;
    for (size_t i = array.size(); i < std::max(array.size(), other.array.size());
        i++)
        if (other.array[i] != 0)
            return true;
    for (size_t i = other.array.size();
        i < std::max(array.size(), other.array.size()); i++)
        if (array[i] != 0)
            return true;
    for (size_t i = 0; i < std::min(array.size(), other.array.size()); i++)
        if (array[i] != other.array[i])
            return true;
    return false;
}

bool Integer::operator==(const Integer& other) const {
    return !(*this != other);
}

Integer& Integer::operator=(const Integer& other) {
    is_negative = other.is_negative;
    array = other.array;
    return *this;
}

Integer& Integer::operator=(Integer&& other) {
    is_negative = other.is_negative;
    array = std::move(other.array);
    return *this;
}

std::string Integer::to_string(void) const {
    if (is_zero())
        return {48};
    // https://en.wikipedia.org/wiki/Double_dabble 참고
    Integer result(0);
    uint64_t mask = 0;
    uint64_t check = 0;
    uint64_t add = 0;
    result.array.resize(((array.size() << 1) / 5 + 1) << 2, 0);
    for (size_t i = array.size(); i > 0; i--) {
        result.array[0] = array[i - 1];
        for (int32_t n = 0; n < 64; n++) {
            for (size_t j = 1; j < result.array.size(); j++) { // 자리 올림 검사
                mask = 0xF000000000000000;
                check = 0x5000000000000000;
                add = 0x3000000000000000;
                for (int32_t m = 0; m < 16; m++) {
                    if ((result.array[j] & mask) >= check)
                        result.array[j] += add;
                    mask >>= 4;
                    check >>= 4;
                    add >>= 4;
                }
            }
            result <<= 1;
        }
    }

    std::string str_result;

    if (!is_zero() && is_negative)
        str_result.push_back(45);

    bool check2 = true;
    uint64_t c = 0;
    for (size_t i = result.array.size(); i > 1; i--) {
        mask = 0xF000000000000000;
        for (size_t j = 16; j > 0; j--) {
            c = ((result.array[i - 1] & mask) >> (4 * (j - 1)));
            mask >>= 4;
            if (c == 0 && check2)
                continue;
            str_result.push_back(c + 48);
            check2 = false;
        }
    }
    return std::move(str_result);
}

size_t Integer::size() const {
    return array.size();
}

Integer Integer::operator*(const Integer& other) const {
    return std::move(this->karatsuba_mul(other));
}

Integer Integer::inverse_of(const Integer& n, const uint64_t l) {
    Integer result(1);
    const uint64_t log2_l = _log2(l);
    const uint64_t r = log2_l + ((l & ((1ULL << log2_l) + ~0ULL)) != 0 ? 1 : 0);
    for (size_t i = 0; i < r; i++)
        result = std::move(mod((result << 1) + mod(Integer::negate((mod(result * result, 1ULL << (i + 1)) * mod(n, 1ULL << (i + 1)))), 1ULL << (i + 1)), 1ULL << (i + 1)));
    return std::move(result);
}

Integer Integer::reverse(const Integer& n, const uint64_t l) {
    const uint64_t q = (l - 1) >> 6;
    const uint32_t r = (l - 1) & 0x3F;
    Integer result;
    result.array.resize(q + 1, 0);
    for (size_t i = 0; i < std::min(n.array.size(), static_cast<size_t>(q + 1)); i++)
        result.array[i] = n.array[i];
    result <<= 63 - r;
    result.array.resize(q + 1, 0);
    for (size_t i = 0; i < (((q + 1) >> 1) + ((q + 1) & 0x1)); i++) {
        const uint64_t c = reverse_64bit(result.array[q - i]);
        result.array[q - i] = reverse_64bit(result.array[i]);
        result.array[i] = c;
    }
    result.normalize();
    return std::move(result);
}

Integer Integer::operator[](std::pair<uint64_t, uint64_t> p) const {
    const size_t q1 = static_cast<size_t>(p.first >> 6);
    const uint64_t r1 = p.first & 0x3F;
    const size_t q2 = static_cast<size_t>(p.second >> 6);
    const uint64_t r2 = p.second & 0x3F;

    Integer result;
    result.array.resize(q2 - q1 + 1, 0); // O(q2 - q1)
    for (size_t i = 0; i < std::min(q2 - q1 + 1, array.size() - q1); i++) // O(q2 - q1)
        result.array[i] = array[i + q1];
    result.array[q2 - q1] &= ((1ULL << r2) + ~0ULL);
    result >>= r1; // O(q2 - q1)
    result.normalize(); // O(q2 - q1)
    return std::move(result);
}

Integer& Integer::operator=(const std::string& str) {
    *this = std::move(Integer(str));
    return *this;
}

Integer Integer::operator%(const Integer& n) const {
    return std::move(*this - ((*this / n) * n));
}

std::ostream& operator<<(std::ostream& cout, const Integer &num) {
    cout << num.to_string();
    return cout;
}

std::istream& operator>>(std::istream& cin, Integer &num) {
    std::string number;
    cin >> number;
    num = std::move(Integer(number));
    return cin;
}

Integer mod_add(const Integer& a, const Integer& b, const Integer& m) {
    const Integer result = std::move(a + b);
    if (m <= result)
        return std::move(result - m);
    return std::move(result);
}

Integer redc(const Integer& T, const Integer& N, const Integer& NN, const uint64_t R) {
    const Integer t = (T + Integer::mod(Integer::mod(T, R) * NN, R) * N) >> R;
    if (N <= t)
        return std::move(t - N);
    return std::move(t);
}

Integer mod_exp2(const uint64_t r, const Integer& N, const Integer& NN, const uint64_t R, const Integer& twoR_mod_N) {
    if (r == 0)
        return 1;
    if (r == 1)
        return twoR_mod_N;
    Integer rR_mod_m = std::move(mod_exp2(r >> 1, N, NN, R, twoR_mod_N));
    rR_mod_m = std::move(redc(rR_mod_m * rR_mod_m, N, NN, R));
    if (r & 0x1ULL)
        rR_mod_m = std::move(redc(rR_mod_m * twoR_mod_N, N, NN, R));
    return std::move(rR_mod_m);
}

Integer mod_odd(const Integer& n, const Integer& m) {
    const uint64_t deg_n = Integer::log2(n);
    const uint64_t deg_m = Integer::log2(m);

    if (deg_n < deg_m)
        return n;
    else if (deg_m == deg_n) {
        if (m <= n)
            return n - m;
        return n;
    } // deg m < deg n

    const Integer R = std::move(Integer(1) << (deg_m + 1)); // O(deg m)
    const Integer neg_inverse_of_m = std::move(R - Integer::inverse_of(m, deg_m + 1)); // O(deg m)
    const Integer R_mod_m = std::move(R - m); // O(deg m)
    const Integer twoR_mod_m = std::move(mod_add(R_mod_m, R_mod_m, m)); // O(deg m)
    const Integer R2_mod_m = std::move(redc(mod_exp2((deg_m + 1) << 1, m, neg_inverse_of_m, deg_m + 1, twoR_mod_m), m, neg_inverse_of_m, deg_m + 1)); // O(M(deg m) log deg m)
    // R^2 R mod m
    uint64_t pos = 0;
    Integer _mul = R_mod_m;
    Integer result = 0;
    while (pos <= deg_n) { // deg n / deg m번 반복
        Integer pn = std::move(n[{pos, pos + deg_m + 1}]); // O(deg m)
        if (m <= pn) // O(deg m)
            pn -= m; // O(deg m)
        // pn mod m
        pn = std::move(redc(pn * R2_mod_m, m, neg_inverse_of_m, deg_m + 1)); // O(M(deg m))
        // pn R mod m
        pn = std::move(redc(pn * _mul, m, neg_inverse_of_m, deg_m + 1)); // O(M(deg m))
        // pn R^i R mod m
        result = std::move(mod_add(result, redc(pn, m, neg_inverse_of_m, deg_m + 1), m)); // O(M(deg m))
        _mul = std::move(redc(_mul * R2_mod_m, m, neg_inverse_of_m, deg_m + 1)); // O(M(deg m))
        pos += deg_m + 1;
    }

    return std::move(result);
}
