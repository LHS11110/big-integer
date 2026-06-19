#pragma once
#include <vector>
#include <iostream>

class Integer {
private:
  bool is_negative;
  std::vector<uint64_t> array;
  Integer& plus_one(const bool ignore_carry = false);
  Integer& plus_(const Integer&, const bool ignore_carry = false);
  Integer& minus_(const Integer&);
  Integer& _minus(Integer&) const;
  bool is_zero(void) const;

public:
  Integer();
  Integer(const uint64_t, const bool = false);
  Integer(const std::vector<uint64_t>&);
  Integer(const Integer&) = default;
  Integer(const Integer&, const bool);
  Integer(Integer&&);
  Integer(Integer&&, const bool);
  Integer(const std::string&);
  Integer(const std::vector<uint64_t>::iterator, const std::vector<uint64_t>::iterator, const uint64_t mask = 0xFFFFFFFFFFFFFFFFULL, const bool _is_negative = false);

  /**
   * @brief 상위 8바이트들 중에서 값이 0인 것을 제거합니다.
   * @return None
   * @note 시간 복잡도: O(n)
   */
  void normalize(void);

  /**
   * @brief log_2를 수행하고 소수점은 버립니다.
   * @param n log n을 구하는 데 사용할 정수
   * @return ⌊log n⌋
   * @note 시간 복잡도: O(n)
   */
  static int64_t log2(const Integer& n);

  /**
   * @brief 0이 아닌 가장 최상위 비트의 인덱스를 반환합니다.
   * @param n 정수
   * @note 시간 복잡도: O(n), n은 비트 개수
   */
  static int64_t MSB(const Integer& n);

  /**
   * @brief 0이 아닌 가장 최하위 비트의 인덱스를 반환합니다.
   * @param n 정수
   * @note 시간 복잡도: O(n), n은 비트 개수
   */
  static int64_t LSB(const Integer& n);

  /**
   * @brief karatsuba 곱셈 알고리즘을 수행합니다.
   * @param rhs 곱할 정수
   * @return a x b
   * @note 시간 복잡도: O(N^(log_2 3)) = O(N^1.58...)
   */
  Integer karatsuba_mul(const Integer& rhs) const;

  /**
   * @brief 절댓값을 취합니다.
   * @param 절댓값을 취할 정수
   * @return |n|
   * @note 시간 복잡도: O(1)
   */
  static Integer& abs(Integer &n);

  /**
   * @brief 절댓값을 취합니다.
   * @param 절댓값을 취할 정수
   * @return |n|
   * @note 시간 복잡도: O(1)
   */
  static Integer abs(Integer &&n);

  /**
   * @brief 절댓값을 취합니다.
   * @param 절댓값을 취할 정수
   * @return |n|
   * @note 시간 복잡도: O(n)
   */
  static Integer abs(const Integer& n);

  /**
   * @brief 부호를 바꿉니다.
   * @param n 부호를 바꿀 Integer 객체
   * @return 부호가 바뀐 결과 : -n
   * @note 시간 복잡도: O(1)
   */
  static Integer& negate(Integer &n) noexcept;

  /**
   * @brief 부호를 바꿉니다.
   * @param n 부호를 바꿀 Integer 객체
   * @return 부호가 바뀐 결과 : -n
   * @note 시간 복잡도: O(1)
   */
  static Integer negate(Integer &&n) noexcept;

  /**
   * @brief 부호에 관계없이 산술 덧셈은 수행합니다.
   * @param rhs 덧셈을 수행할 Integer 객체
   * @param ignore_carry 자리 올림 무시 여부
   * @return |left| + |right|
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer plus(const Integer& rhs, const bool ignore_carry = false) const;

  /**
   * @brief 부호에 관계없이 산술 뺄셈은 수행합니다.
   * @param rhs 뺄셈을 수행할 Integer 객체
   * @return |left| - |right|
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer minus(const Integer& rhs) const;

  /**
   * @brief 부호를 바꿉니다.
   * @return 부호가 바뀐 결과
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator-(void) const;

  /**
   * @brief 비트열의 모든 비트를 반전(NOT 연산)합니다.
   * @return Integer 모든 비트가 반전된 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator~(void) const;

  /**
   *  @brief left-bit-shift 연산을 수행합니다.
   * @param shift left-shift할 횟수 
   * @return lshift가 수행된 새로운 Integer 객체
   * @note 시간 복잡도: O(n + k) (n은 비트의 길이)
   */
  Integer operator<<(const uint64_t shift) const;

  /**
   *  @brief right-bit-shift 연산을 수행합니다.
   * @param shift right-shift할 횟수 
   * @return rshift가 수행된 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator>>(const uint64_t shift) const;

  /**
   * @brief left-bit-shift 연산을 수행한 후 저장합니다.
   * @param shift left-shift할 횟수 
   * @return this
   * @note 시간 복잡도: O(n + k) (n은 비트의 길이)
   */
  Integer& operator<<=(const uint64_t shift);

  /**
   * @brief right-bit-shift 연산을 수행한 후 저장합니다.
   * @param shift right-shift할 횟수 
   * @return this
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer& operator>>=(const uint64_t shift);

  /**
   * @brief 산술 덧셈을 수행합니다.
   * @param rhs 더할 우항(Right-Hand Side) 정수 객체
   * @return left + right
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator+(const Integer& rhs) const;

  /**
   * @brief 산술 덧셈을 수행한 후 결과를 본 객체에 저장합니다.
   * @param rhs 더할 우항(Right-Hand Side) 정수 객체
   * @return left + right
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer& operator+=(const Integer& rhs);

  /**
   * @brief 산술 뺄셈을 합니다.
   * 
   * 현재 정수와 전달받은 정수의 뺄셈을 계산하여 새로운 Integer 객체를 반환합니다.
   * 피연산자들의 원본 상태는 변경되지 않습니다.
   * 
   * @param rhs 뺄 우항(Right-Hand Side) 정수 객체
   * @return this
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator-(const Integer& rhs) const;

  /**
   * @brief 산술 뺄셈을 수행한 후 결과를 본 객체에 저장합니다.
   * @param rhs 뺄 우항(Right-Hand Side) 정수 객체
   * @return this
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer& operator-=(const Integer& rhs);

  /**
   * @brief 크기를 비교합니다.
   * @param rhs 비교할 우항 정수 객체
   * @return 비교할 객체보다 작거나 같은지에 대한 여부
   * @note 시간 복잡도: O(n)
   */
  bool operator<=(const Integer& rhs) const;

  /**
   * @brief 크기를 비교합니다.
   * @param rhs 비교할 우항 정수 객체
   * @return 비교할 객체보다 작은지 대한 여부
   * @note 시간 복잡도: O(n)
   */
  bool operator<(const Integer& rhs) const;

  /**
   * @brief 두 정수가 다른지 비교합니다.
   * @param rhs 비교할 우항 정수 객체
   * @return 다른지에 대한 여부
   * @note 시간 복잡도: O(n)
   */
  bool operator!=(const Integer& rhs) const;

  /**
   * @brief 두 정수가 같은지 비교합니다.
   * @param rhs 비교할 정수
   * @return 같은지에 대한 여부
   * @note 시간 복잡도: O(n)
   */
  bool operator==(const Integer& rhs) const;

  Integer& operator=(const Integer&);
  Integer& operator=(Integer&&);

  /**
   * @brief 주어진 정수를 10진법으로 구성된 문자열로 변환합니다.
   * @return 10진법으로 구성된 문자열
   * @note 시간 복잡도: O(n^2) (n은 비트의 길이)
   */
  std::string to_string(void) const;

  /**
   * @brief 정수를 구성하는 벡터의 사이즈를 반환합니다.
   * @return array.size()
   * @note 시간 복잡도: O(1)
   */
  size_t size(void) const;

  /**
   * @brief 두 정수의 곱을 구합니다.
   * @param rhs 곱셈에 사용할 정수
   * @return 두 정수의 곱: a * b
   * @note 시간 복잡도: O(M(n)), (M(n)은 곱셈 알고리즘의 시간 복잡도이며, 여기서는 카라추바 알고리즘과 동일)
   */
  Integer operator*(const Integer& rhs) const;

  /**
   * @brief 모듈러 2^l에 대한 n의 역원을 구합니다. 단, n은 반드시 홀수이어야 합니다.
   * @param n 정수
   * @param l 64비트 정수
   * @return 모듈러 2^l에 대한 역원 n^-1
   * @note 시간 복잡도: O(M(l)), 여기서 M(n)은 곱셈에 대한 시간 복잡도
   */
  static Integer inverse_of(const Integer& n, const uint64_t l);

  /**
   * @brief 주어진 정수에 대한 n mod 2^l을 구합니다.
   * @param n 정수
   * @param l 64비트 정수
   * @return n mod 2^l
   * @note 시간 복잡도: O(l)
   */
  static Integer mod(const Integer& n, const uint64_t l);

  /**
   * @brief 몫(this / n)을 구합니다.
   * @param n 정수
   * @return this / n
   * @note 
   */
  Integer operator/(const Integer& n) const;

  /**
   * @brief l개의 비트를 뒤집습니다. 단, l비트보다 더 많은 비트가 정수로 들어오면 나머지는 버립니다.
   * @param n 정수
   * @return rev(n)
   * @note 시간 복잡도: O(l)
   */
  static Integer reverse(const Integer& n, const uint64_t l);

  /**
   * @brief 특정 구간의 비트열을 가져옵니다.
   * @param p 비트의 인덱스를 담고 있는 순서쌍
   * @return p.first번째부터 p.second - 1번째까지의 비트열
   * @note 시간 복잡도: O(n), n은 가져올 비트의 개수
   */
  Integer operator[](std::pair<uint64_t, uint64_t> p) const;

  Integer& operator=(const std::string& str);
};

std::ostream& operator<<(std::ostream&, const Integer &);
std::istream& operator>>(std::istream&, Integer &);

/**
 * @brief a + b mod m을 구합니다.
 * @param a 0 <= a < m인 정수
 * @param b 0 <= b < m인 정수
 * @param m 정수
 * @return a + b mod m
 * @note 시간 복잡도: O(m), m은 비트 개수
 */
Integer mod_add(const Integer& a, const Integer& b, const Integer& m);

/**
 * @param T 음이 아닌 정수
 * @param N 음이 아닌 정수
 * @param NN 법 2^R에 대한 N의 역원에 음의 부호를 취한 정수: -N^-1 mod 2^R
 * @param R 부호없는 64비트 정수
 * @return T * 2^-R mod N
 * @note 시간 복잡도: O(M(R))
 */
Integer redc(const Integer& T, const Integer& N, const Integer& NN, const uint64_t R);

/**
 * @param r 부호없는 64비트 정수
 * @param N 음이 아닌 정수
 * @param NN 법 2^R에 대한 N의 역원에 음의 부호를 취한 정수: -N^-1 mod 2^R
 * @param R 부호없는 64비트 정수
 * @param twoR_mod_N 2 * 2^R mod N인 정수
 * @return 2^r * 2^R mod N
 * @note 시간 복잡도: O(M(R) log r)
 */
Integer mod_exp2(const uint64_t r, const Integer& N, const Integer& NN, const uint64_t R, const Integer& twoR_mod_N);

/**
 * @param n 음이 아닌 정수
 * @param m 음이 아닌 홀수인 정수
 * @return n mod m
 * @note 시간 복잡도: O(n^log_2 3 log_2 n)
 */
Integer mod_odd(const Integer& n, const Integer& m);
