#define private public
#include "bigint.hpp"
#undef private

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

// 쉼표로 구분된 문자열을 std::vector<uint64_t>로 변환
std::vector<uint64_t> parse_words(const std::string& str) {
    std::vector<uint64_t> result;
    if (str.empty() || str == "empty" || str == "0") {
        result.push_back(0);
        return result;
    }
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, ',')) {
        if (!item.empty()) {
            result.push_back(std::stoull(item));
        }
    }
    return result;
}

// Integer 객체를 "부호 word0,word1,..." 형식으로 출력
void print_integer(const Integer& n) {
    std::cout << (n.is_negative ? "-" : "+") << " ";
    for (size_t i = 0; i < n.array.size(); ++i) {
        std::cout << n.array[i];
        if (i + 1 < n.array.size()) std::cout << ",";
    }
    std::cout << std::endl;
}

// "is_negative,word0,word1,..." 포맷의 문자열로 Integer 객체를 파싱 생성
Integer parse_integer(const std::string& neg_str, const std::string& words_str) {
    bool is_neg = (neg_str == "true" || neg_str == "1" || neg_str == "-");
    std::vector<uint64_t> words = parse_words(words_str);
    Integer n(words);
    n.is_negative = is_neg;
    return n;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <method> [args...]" << std::endl;
        return 1;
    }
    
    std::string method = argv[1];
    
    if (method == "construct_default") {
        Integer n;
        print_integer(n);
    }
    else if (method == "construct_uint64") {
        uint64_t val = std::stoull(argv[2]);
        bool is_neg = (std::string(argv[3]) == "true");
        Integer n(val, is_neg);
        print_integer(n);
    }
    else if (method == "construct_vector") {
        std::vector<uint64_t> words = parse_words(argv[2]);
        Integer n(words);
        print_integer(n);
    }
    else if (method == "construct_copy") {
        Integer src = parse_integer(argv[2], argv[3]);
        bool new_is_neg = (std::string(argv[4]) == "true");
        Integer n(src, new_is_neg);
        print_integer(n);
    }
    else if (method == "construct_move") {
        Integer src = parse_integer(argv[2], argv[3]);
        Integer n(std::move(src));
        print_integer(n);
    }
    else if (method == "construct_move_sign") {
        // args: <is_neg> <words> <new_is_neg>
        Integer src = parse_integer(argv[2], argv[3]);
        bool new_is_neg = (std::string(argv[4]) == "true");
        Integer n(std::move(src), new_is_neg);
        print_integer(n);
    }
    else if (method == "construct_string") {
        // args: <string_val>
        std::string str_val = argv[2];
        try {
            Integer n(str_val);
            print_integer(n);
        } catch (const std::invalid_argument& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    else if (method == "mod10") {
        // args: <is_neg> <words>
        Integer n = parse_integer(argv[2], argv[3]);
        std::cout << n.mod10() << std::endl;
    }
    else if (method == "negate_lvalue") {
        Integer n = parse_integer(argv[2], argv[3]);
        Integer::negate(n);
        print_integer(n);
    }
    else if (method == "negate_rvalue") {
        Integer n = parse_integer(argv[2], argv[3]);
        Integer result = Integer::negate(std::move(n));
        print_integer(result);
    }
    else if (method == "plus") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        bool ignore_carry = (std::string(argv[6]) == "true");
        Integer result = n1.plus(n2, ignore_carry);
        print_integer(result);
    }
    else if (method == "minus") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        Integer result = n1.minus(n2);
        print_integer(result);
    }
    else if (method == "operator_negate") {
        Integer n = parse_integer(argv[2], argv[3]);
        Integer result = -n;
        print_integer(result);
    }
    else if (method == "operator_tilde") {
        Integer n = parse_integer(argv[2], argv[3]);
        Integer result = ~n;
        print_integer(result);
    }
    else if (method == "operator_lshift") {
        Integer n = parse_integer(argv[2], argv[3]);
        uint64_t shift = std::stoull(argv[4]);
        Integer result = n << shift;
        print_integer(result);
    }
    else if (method == "operator_rshift") {
        // args: <is_neg> <words> <shift>
        Integer n = parse_integer(argv[2], argv[3]);
        uint64_t shift = std::stoull(argv[4]);
        Integer result = n >> shift;
        print_integer(result);
    }
    else if (method == "operator_lshift_assign") {
        // args: <is_neg> <words> <shift>
        Integer n = parse_integer(argv[2], argv[3]);
        uint64_t shift = std::stoull(argv[4]);
        n <<= shift;
        print_integer(n);
    }
    else if (method == "operator_rshift_assign") {
        // args: <is_neg> <words> <shift>
        Integer n = parse_integer(argv[2], argv[3]);
        uint64_t shift = std::stoull(argv[4]);
        n >>= shift;
        print_integer(n);
    }
    else if (method == "operator_plus") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        Integer result = n1 + n2;
        print_integer(result);
    }
    else if (method == "operator_plus_assign") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        n1 += n2;
        print_integer(n1);
    }
    else if (method == "operator_minus") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        Integer result = n1 - n2;
        print_integer(result);
    }
    else if (method == "operator_minus_assign") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        n1 -= n2;
        print_integer(n1);
    }
    else if (method == "operator_le") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        std::cout << (n1 <= n2 ? "true" : "false") << std::endl;
    }
    else if (method == "operator_ne") {
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        std::cout << (n1 != n2 ? "true" : "false") << std::endl;
    }
    else if (method == "operator_eq") {
        // args: <is_neg1> <words1> <is_neg2> <words2>
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        std::cout << (n1 == n2 ? "true" : "false") << std::endl;
    }
    else if (method == "operator_assign_copy") {
        // args: <is_neg1> <words1> <is_neg2> <words2>
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        n1 = n2;
        print_integer(n1);
    }
    else if (method == "operator_assign_move") {
        // args: <is_neg1> <words1> <is_neg2> <words2>
        Integer n1 = parse_integer(argv[2], argv[3]);
        Integer n2 = parse_integer(argv[4], argv[5]);
        n1 = std::move(n2);
        print_integer(n1);
    }
    else if (method == "to_string") {
        // args: <is_neg> <words>
        Integer n = parse_integer(argv[2], argv[3]);
        std::cout << n.to_string() << std::endl;
    }
    else if (method == "ostream_out") {
        // args: <is_neg> <words>
        Integer n = parse_integer(argv[2], argv[3]);
        std::cout << n << std::endl;
    }
    else {
        std::cerr << "Unknown method: " << method << std::endl;
        return 1;
    }
    
    return 0;
}
