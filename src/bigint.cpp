#include "bigint.hpp"
#define CFLAG(a, b, mask) (((a) & (b) & (mask)) || ((((a) | (b)) & (mask)) & ~(((a) + (b)) & (mask))))
#define MASK 0x8000000000000000ULL
#define MASK2 0xFFFFFFFFFFFFFFFFULL

Integer::Integer() : is_negative(false), array({0}) {}

Integer::Integer(const uint64_t num) : is_negative(false), array({num}) {}

Integer::Integer(const std::vector<uint64_t>& arr) : is_negative(false), array(arr) {
    if (array.empty())
        array.push_back(0);
}

Integer Integer::plus(const Integer& other) const {
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

    if (carry)
        result.array.push_back(1);

    return result;
}

Integer Integer::minus(const Integer& other) const {
    Integer result;
    const uint64_t this_len = array.size();
    const uint64_t other_len = other.array.size();
    const uint64_t len = std::max(this_len, other_len) + 1; // +1은 여유 공간
    result.array.resize(len, MASK2);
    for (uint64_t i = 0; i < len; i++) // O(n)의 시간 복잡도로 비트 반전 수행
        result.array[i] = ~other.array[i]; // NOT other.array[i]
    result = result.plus(1); // O(n)의 시간 복잡도로 산술 덧셈 수행
    result.array.pop_back(); // 자리 올림 제거
    return result.plus(*this); // O(n)의 시간 복잡도로 산술 덧셈 수행
}

Integer Integer::operator~() const {
    Integer result;
    result.array.resize(array.size(), 0);
    for (size_t i = 0; i < array.size(); i++)
        result.array[i] = ~array[i];
    return result;
}

std::string Integer::to_string() const {
    if (array.empty())
        return "0";
    std::string str;
    for (auto it = array.rbegin(); it != array.rend(); ++it) {
        std::string part = std::to_string(*it);
        if (it != array.rbegin()) {
            // 각 부분을 16자리로 맞추기 위해 앞에 0을 추가
            if (part.length() < 16) {
                part = std::string(16 - part.length(), '0') + part;
            }
        }
        str += part;
    }
    // 불필요한 0 제거
    size_t non_zero_idx = str.find_first_not_of('0');
    if (non_zero_idx == std::string::npos) {
        return "0";
    }
    str.erase(0, non_zero_idx);
    return is_negative ? "-" + str : str;
}
