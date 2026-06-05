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
  void normalize(void);
  uint64_t max_bit(void) const;

public:
  Integer();
  Integer(const uint64_t, const bool = false);
  Integer(const std::vector<uint64_t>&);
  Integer(const Integer&, const bool b = false);
  Integer(Integer&&);
  Integer(Integer&&, const bool);
  Integer(const std::string&);

  Integer karatsuba_mul(const Integer&) const;

  /**
   * @brief 부호를 바꿉니다. 단, 이 함수는 전달받은 객체의 상태를 변경시킵니다.
   * @param n 부호를 바꿀 Integer 객체
   * @return 부호가 바뀐 결과 : -n
   * @note 시간 복잡도 : O(1)
   */
  static Integer& negate(Integer &n) noexcept;
  static Integer negate(Integer &&n) noexcept;

  /**
   * @brief 부호에 관계없이 산술 덧셈은 수행합니다.
   * 
   * 본 객체(this)의 상태는 변경되지 않습니다.
   * 
   * @param rhs 덧셈을 수행할 Integer 객체
   * @param ignore_carry 자리 올림 무시 여부
   * @return |left| + |right|
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer plus(const Integer& rhs, const bool ignore_carry = false) const;

  /**
   * @brief 부호에 관계없이 산술 뺄셈은 수행합니다.
   * 
   * 본 객체(this)의 상태는 변경되지 않습니다.
   * 
   * @param rhs 뺄셈을 수행할 Integer 객체
   * @return |left| - |right|
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer minus(const Integer& rhs) const;

  /**
   * @brief 부호를 바꿉니다. 단, 본 객체의 상태는 변경되지 않습니다.
   * @return 부호가 바뀐 결과
   * @note 시간 복잡도 : O(n) (n은 비트의 길이)
   */
  Integer operator-(void) const;

  /**
   * @brief 비트열의 모든 비트를 반전(NOT 연산)합니다.
   * 
   * 기존 비트열의 0은 1로, 1은 0으로 반전된 새로운 Integer 객체를 반환합니다.
   * 본 객체(this)의 상태는 변경되지 않습니다.
   * 
   * @return Integer 모든 비트가 반전된 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator~(void) const;

  /**
   *  @brief left-bit-shift 연산을 수행합니다.
   * 
   * @param shift left-shift할 횟수 
   * @return lshift가 수행된 새로운 Integer 객체
   * @note 시간 복잡도 : O(n + m) (n은 비트의 길이, m은 시프트 횟수)
   */
  Integer operator<<(const uint64_t shift) const;

  /**
   *  @brief right-bit-shift 연산을 수행합니다.
   * 
   * @param shift right-shift할 횟수 
   * @return rshift가 수행된 새로운 Integer 객체
   * @note 시간 복잡도 : O(n) (n은 비트의 길이)
   */
  Integer operator>>(const uint64_t shift) const;

  /**
   *  @brief left-bit-shift 연산을 수행한 후 저장합니다.
   * 
   * @param shift left-shift할 횟수 
   * @return this
   * @note 시간 복잡도 : O(n + m) (n은 비트의 길이, m은 시프트 횟수)
   */
  Integer& operator<<=(const uint64_t shift);

  /**
   *  @brief right-bit-shift 연산을 수행한 후 저장합니다.
   * 
   * @param shift right-shift할 횟수 
   * @return this
   * @note 시간 복잡도 : O(n) (n은 비트의 길이)
   */
  Integer& operator>>=(const uint64_t shift);

  /**
   * @brief 산술 덧셈을 합니다.
   * 
   * 현재 정수와 전달받은 정수의 합을 계산하여 새로운 Integer 객체를 반환합니다.
   * 피연산자들의 원본 상태는 변경되지 않습니다.
   * 
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
   * @note 시간 복잡도 : O(n)
   */
  bool operator<=(const Integer& rhs) const;

  /**
   * @brief 크기를 비교합니다.
   * @param rhs 비교할 우항 정수 객체
   * @return 비교할 객체보다 작은지 대한 여부
   * @note 시간 복잡도 : O(n)
   */
  bool operator<(const Integer& rhs) const;

  /**
   * @brief 두 정수가 다른지 비교합니다.
   * @param rhs 비교할 우항 정수 객체
   * @return 다른지에 대한 여부
   * @note 시간 복잡도 : O(n)
   */
  bool operator!=(const Integer& rhs) const;

  bool operator==(const Integer& rhs) const;

  Integer& operator=(const Integer&);
  Integer& operator=(Integer&&);

  /**
   * @brief 주어진 정수를 10진법으로 구성된 문자열로 변환합니다.
   * @return 10진법으로 구성된 문자열
   * @note 시간 복잡도 : O(n^2) (n은 비트의 길이)
   */
  std::string to_string(void) const;

  size_t size(void) const;
};

std::ostream& operator<<(std::ostream&, const Integer &);
std::istream& operator>>(std::istream&, Integer &);
