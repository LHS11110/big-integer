#include "bigint.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// 쉼표로 구분된 문자열을 std::vector<uint64_t>로 변환
std::vector<uint64_t> parse_words(const std::string &str) {
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

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <method> [args...]" << std::endl;
    return 1;
  }

  std::string method = argv[1];

  if (method == "construct_default") {
    Integer n;
    std::cout << n << std::endl;
  } else if (method == "construct_uint64") {
    uint64_t val = std::stoull(argv[2]);
    bool is_neg = (std::string(argv[3]) == "true");
    Integer n(val, is_neg);
    std::cout << n << std::endl;
  } else if (method == "construct_vector") {
    std::vector<uint64_t> words = parse_words(argv[2]);
    Integer n(words);
    std::cout << n << std::endl;
  } else if (method == "construct_copy") {
    Integer src(argv[2]);
    bool new_is_neg = (std::string(argv[3]) == "true");
    Integer n(src, new_is_neg);
    std::cout << n << std::endl;
  } else if (method == "construct_move") {
    Integer src(argv[2]);
    Integer n(std::move(src));
    std::cout << n << std::endl;
  } else if (method == "construct_move_sign") {
    Integer src(argv[2]);
    bool new_is_neg = (std::string(argv[3]) == "true");
    Integer n(std::move(src), new_is_neg);
    std::cout << n << std::endl;
  } else if (method == "construct_string") {
    std::string str_val = argv[2];
    try {
      Integer n(str_val);
      std::cout << n << std::endl;
    } catch (const std::invalid_argument &e) {
      std::cout << "ERROR: " << e.what() << std::endl;
    }
  } else if (method == "negate_lvalue") {
    Integer n(argv[2]);
    Integer::negate(n);
    std::cout << n << std::endl;
  } else if (method == "negate_rvalue") {
    Integer n(argv[2]);
    Integer result = Integer::negate(std::move(n));
    std::cout << result << std::endl;
  } else if (method == "plus") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    bool ignore_carry = (std::string(argv[4]) == "true");
    Integer result = n1.plus(n2, ignore_carry);
    std::cout << result << std::endl;
  } else if (method == "minus") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    Integer result = n1.minus(n2);
    std::cout << result << std::endl;
  } else if (method == "karatsuba_mul") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    Integer result = n1.karatsuba_mul(n2);
    std::cout << result << std::endl;
  } else if (method == "operator_negate") {
    Integer n(argv[2]);
    Integer result = -n;
    std::cout << result << std::endl;
  } else if (method == "operator_tilde") {
    Integer n(argv[2]);
    Integer result = ~n;
    std::cout << result << std::endl;
  } else if (method == "operator_lshift") {
    Integer n(argv[2]);
    uint64_t shift = std::stoull(argv[3]);
    Integer result = n << shift;
    std::cout << result << std::endl;
  } else if (method == "operator_rshift") {
    Integer n(argv[2]);
    uint64_t shift = std::stoull(argv[3]);
    Integer result = n >> shift;
    std::cout << result << std::endl;
  } else if (method == "operator_lshift_assign") {
    Integer n(argv[2]);
    uint64_t shift = std::stoull(argv[3]);
    n <<= shift;
    std::cout << n << std::endl;
  } else if (method == "operator_rshift_assign") {
    Integer n(argv[2]);
    uint64_t shift = std::stoull(argv[3]);
    n >>= shift;
    std::cout << n << std::endl;
  } else if (method == "operator_plus") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    Integer result = n1 + n2;
    std::cout << result << std::endl;
  } else if (method == "operator_plus_assign") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    n1 += n2;
    std::cout << n1 << std::endl;
  } else if (method == "operator_minus") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    Integer result = n1 - n2;
    std::cout << result << std::endl;
  } else if (method == "operator_minus_assign") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    n1 -= n2;
    std::cout << n1 << std::endl;
  } else if (method == "operator_le") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    std::cout << (n1 <= n2 ? "true" : "false") << std::endl;
  } else if (method == "operator_ne") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    std::cout << (n1 != n2 ? "true" : "false") << std::endl;
  } else if (method == "operator_eq") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    std::cout << (n1 == n2 ? "true" : "false") << std::endl;
  } else if (method == "operator_lt") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    std::cout << (n1 < n2 ? "true" : "false") << std::endl;
  } else if (method == "abs_lvalue") {
    Integer n(argv[2]);
    Integer::abs(n);
    std::cout << n << std::endl;
  } else if (method == "abs_rvalue") {
    Integer n(argv[2]);
    Integer result = Integer::abs(std::move(n));
    std::cout << result << std::endl;
  } else if (method == "size") {
    Integer n(argv[2]);
    std::cout << n.size() << std::endl;
  } else if (method == "operator_assign_copy") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    n1 = n2;
    std::cout << n1 << std::endl;
  } else if (method == "operator_assign_move") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    n1 = std::move(n2);
    std::cout << n1 << std::endl;
  } else if (method == "to_string") {
    Integer n(argv[2]);
    std::cout << n.to_string() << std::endl;
  } else if (method == "ostream_out") {
    Integer n(argv[2]);
    std::cout << n << std::endl;
  } else if (method == "operator_mul") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    Integer result = n1 * n2;
    std::cout << result << std::endl;
  } else if (method == "operator_div") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    try {
      Integer result = n1 / n2;
      std::cout << result << std::endl;
    } catch (const std::invalid_argument &e) {
      std::cout << "ERROR: " << e.what() << std::endl;
    }
  } else if (method == "log2") {
    Integer n(argv[2]);
    std::cout << Integer::log2(n) << std::endl;
  } else if (method == "msb") {
    Integer n(argv[2]);
    std::cout << Integer::MSB(n) << std::endl;
  } else if (method == "lsb") {
    Integer n(argv[2]);
    std::cout << Integer::LSB(n) << std::endl;
  } else if (method == "mod") {
    Integer n(argv[2]);
    uint64_t l = std::stoull(argv[3]);
    Integer result = Integer::mod(n, l);
    std::cout << result << std::endl;
  } else if (method == "inverse_of") {
    Integer n(argv[2]);
    uint64_t l = std::stoull(argv[3]);
    Integer result = Integer::inverse_of(n, l);
    std::cout << result << std::endl;
  } else if (method == "reverse") {
    Integer n(argv[2]);
    uint64_t l = std::stoull(argv[3]);
    Integer result = Integer::reverse(n, l);
    std::cout << result << std::endl;
  } else if (method == "operator_subscript") {
    Integer n(argv[2]);
    uint64_t first = std::stoull(argv[3]);
    uint64_t second = std::stoull(argv[4]);
    Integer result = n[{first, second}];
    std::cout << result << std::endl;
  } else if (method == "istream_in") {
    std::string input_str = argv[2];
    std::istringstream iss(input_str);
    Integer n;
    iss >> n;
    std::cout << n << std::endl;
  } else if (method == "mod_add") {
    Integer a(argv[2]);
    Integer b(argv[3]);
    Integer m(argv[4]);
    Integer result = mod_add(a, b, m);
    std::cout << result << std::endl;
  } else if (method == "mod_odd") {
    Integer n(argv[2]);
    Integer m(argv[3]);
    Integer result = mod_odd(n, m);
    std::cout << result << std::endl;
  } else if (method == "operator_mod") {
    Integer n1(argv[2]);
    Integer n2(argv[3]);
    try {
      Integer result = n1 % n2;
      std::cout << result << std::endl;
    } catch (const std::invalid_argument &e) {
      std::cout << "ERROR: " << e.what() << std::endl;
    }
  } else {
    std::cerr << "Unknown method: " << method << std::endl;
    return 1;
  }

  return 0;
}
