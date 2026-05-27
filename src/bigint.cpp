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
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
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
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
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
    while (array.size() > 1 && array.back() == 0)
        array.pop_back();
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
        if (i < this_len) {
            carry = array[i] == MASK2 && carry;
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
    while (other.array.size() > 1 && other.array.back() == 0)
        other.array.pop_back();
    return other;
}

bool Integer::is_zero() const {
    for (uint64_t num : array)
        if (num != 0)
            return false;
    return true;
}

uint32_t Integer::mod10() const {
    uint32_t result = 0;
    for (size_t i = 0; i < array.size(); i++)
        result = (result + ((array[i] % 10) * (i == 0 ? 1 : 6))) % 10; // f(a * 2^i) = f(a) * f(2^i)
    return result;
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
        if (i < this_len) {
        carry = array[i] == MASK2 && carry;
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
    while (result.array.size() > 1 &&
            result.array.back() == 0) // 최상위 8바이트가 0이면 제거하여 공간 절약
        result.array.pop_back();
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
    for (int64_t i = result.array.size() - 1; i > 0; i--) {
        result.array[i] <<= q;
        result.array[i] |=
            result.array[i - 1] >> shift; // 아래 주석과 결과가 동일함
        // result.array[i] += (result.array[i - 1] & mask) >> shift;
    }

    result.array[0] <<= q;
    return std::move(result);
}

Integer Integer::operator>>(const uint64_t c) const {
    const uint64_t r = c >> 6;     // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    if (array.size() <= r)
        return std::move(Integer());
    Integer result;
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
    while (result.array.size() > 1 && result.array.back())
        result.array.pop_back();
    return std::move(result);
}

Integer& Integer::operator<<=(const uint64_t c) {
    const uint64_t r = c >> 6;     // c / 64
    const int64_t q = c & 0x3FULL; // c % 64
    array.resize(array.size() + r, 0);
    for (size_t i = array.size(); i > r; i--)
        array[i - 1] = array[i - r - 1];
    const uint64_t mask = (~(MASK >> (q - 1)) + 1); // 비트 마스크
    const uint32_t shift = 64 - q;
    if (array.back() & mask) // 시프트 연산으로 인해 범위를 벗어나는 비트가 있는 경우
        array.push_back(0);
    for (int64_t i = array.size() - 1; i > 0; i--) {
        array[i] <<= q;
        array[i] |=
            (array[i - 1] & mask) >> shift;
    }
    array[0] <<= q;
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
    while (array.size() > 1 && array.back() == 0)
        array.pop_back();
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

std::ostream& operator<<(std::ostream& cout, const Integer &num) {
    cout << num.to_string();
    return cout;
}
