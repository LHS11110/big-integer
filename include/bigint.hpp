#pragma once
#include <vector>

class Integer {
private:
  bool is_negative;
  std::vector<uint64_t> array;

public:
  Integer();
  Integer(const uint64_t);
  Integer(const std::vector<uint64_t>&);

  /**
   * @brief 부호에 관계없이 산술 덧셈은 수행합니다.
   * 
   * 본 객체(this)의 상태는 변경되지 않습니다.
   * 
   * @return Integer 부호에 관계없이 산술 덧셈 결과가 담긴 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer plus(const Integer&) const;

  /**
   * @brief 부호에 관계없이 산술 뺄셈은 수행합니다.
   * 
   * 본 객체(this)의 상태는 변경되지 않습니다.
   * 
   * @return Integer 부호에 관계없이 산술 뺄셈 결과가 담긴 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer minus(const Integer&) const;

  /**
   * @brief 비트열의 모든 비트를 반전(NOT 연산)합니다.
   * 
   * 기존 비트열의 0은 1로, 1은 0으로 반전된 새로운 Integer 객체를 반환합니다.
   * 본 객체(this)의 상태는 변경되지 않습니다.
   * 
   * @return Integer 모든 비트가 반전된 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
  Integer operator~() const;

  std::string to_string() const;

  /**
   * @brief 산술 덧셈합니다.
   * 
   * 현재 정수와 전달받은 정수의 합을 계산하여 새로운 Integer 객체를 반환합니다.
   * 피연산자들의 원본 상태는 변경되지 않습니다.
   * 
   * @param rhs 더할 우항(Right-Hand Side) 정수 객체
   * @return Integer 두 정수의 산술 덧셈 결과가 담긴 새로운 Integer 객체
   * @note 시간 복잡도: O(n) (n은 비트의 길이)
   */
};
