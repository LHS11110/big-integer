#include "bigint.hpp"
#include <iostream>
#define CFLAG(a, b, mask) (((a) & (b) & (mask)) || ((((a) | (b)) & (mask)) & ~(((a) + (b)) & (mask))))
#define MASK 0x8000000000000000ULL
#define MASK2 0xFFFFFFFFFFFFFFFFULL

Integer& Integer::plus_(const Integer& other, bool ignore_carry) {
    bool carry = 0;
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

Integer::Integer() : is_negative(false), array({0}) {}

Integer::Integer(const uint64_t num) : is_negative(false), array({num}) {}

Integer::Integer(const std::vector<uint64_t>& arr) : is_negative(false), array(arr) {
    if (array.empty())
        array.push_back(0);
}

Integer Integer::plus(const Integer& other, bool ignore_carry) const {
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
    result.plus_(1, true); // 1을 더하여 2의 보수로 변환
    result.plus_(*this, true); // this - other
    if (result.array.back() & MASK) { // 음수라면 이를 다시 양수로 바꾸기 위해
        for (uint64_t i = 0; i < len; i++) // O(n)의 시간 복잡도로 비트 반전 수행
            result.array[i] = ~result.array[i]; // NOT result.array[i]
        result.plus_(1, true); // 1을 더하여 2의 보수로 변환
        result.is_negative = true; // 음수임을 표시
    }
    if (result.array.back() == 0) // 최상위 8바이트가 0이면 제거하여 공간 절약
        result.array.pop_back();
    return result;
}

Integer Integer::operator~() const {
    Integer result;
    result.array.resize(array.size(), 0);
    for (size_t i = 0; i < array.size(); i++)
        result.array[i] = ~array[i];
    return result;
}

void Integer::print() const {
    if (is_negative)
        std::cout << "-";
    for (size_t i = array.size(); i > 0; i--)
        std::cout << array[i - 1] << " ";
    std::cout << std::dec << std::endl;
}
