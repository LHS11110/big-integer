# 🔢 BigInteger C++ Library

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg?style=flat-square)](LICENSE)

임의 정밀도(Arbitrary-precision) 정수 연산을 지원하는 C++17 기반의 대용량 정수(BigInteger) 처리 라이브러리입니다. `uint64_t` 배열(Word 단위)을 활용하여 물리적인 메모리 한계까지 확장 가능한 정수를 표현하고 안정적인 산술 연산을 제공합니다.

---

## 📂 프로젝트 구조

```text
.
├── include/
│   └── bigint.hpp        # 라이브러리 헤더 파일 (Integer 클래스 정의)
├── src/
│   └── bigint.cpp        # 라이브러리 소스 코드 (핵심 연산 및 변환 구현)
├── tests/
│   ├── test_runner.cpp   # C++ 테스트 러너 (CLI 기반 메서드 호출 인터페이스)
│   └── test_bigint.py    # Python 기반 단위 테스트 스크립트
├── .gitignore            # Git 추적 제외 파일 목록
└── LICENSE               # Apache 2.0 라이선스 파일
```

---

## 🛠️ 빌드 방법 (Build Guide)

이 라이브러리는 별도의 외부 종속성(Dependency) 없이 **C++17 표준 라이브러리**만으로 빌드할 수 있습니다.

### 1. 라이브러리만 빌드하는 방법 (Compile Library Only)

프로젝트에 라이브러리를 포함시키거나 배포하기 위해 소스 파일(`bigint.cpp`)을 목적 파일(`object file`) 또는 정적 라이브러리(`.a`)로 빌드할 수 있습니다.

#### 목적 파일(.o) 생성
```bash
g++ -std=c++17 -Iinclude -c src/bigint.cpp -o bigint.o
```

#### 정적 라이브러리(.a) 아카이브 생성
```bash
ar rcs libbigint.a bigint.o
```
*생성된 `libbigint.a` 라이브러리와 `include/bigint.hpp` 헤더를 활용해 다른 C++ 프로젝트에서 대용량 정수 연산을 손쉽게 링크하여 사용할 수 있습니다.*

---

### 2. 테스트 빌드 및 실행 방법 (Build & Run Tests)

테스트 시스템은 **C++ 테스트 러너**와 **Python 테스트 스크립트**의 2단 구조로 구성되어 있습니다. C++ 테스트 러너는 CLI 인터페이스를 통해 개별 메서드를 호출하고, Python 스크립트가 이를 자동으로 실행하여 결과를 검증합니다.

#### 테스트 러너 컴파일
```bash
g++ -std=c++17 -Iinclude src/bigint.cpp tests/test_runner.cpp -o test_runner
```

#### Python 테스트 실행
```bash
python3 tests/test_bigint.py
```

*Python 테스트 스크립트는 내부적으로 컴파일된 `test_runner` 바이너리를 호출하여 각 메서드의 입출력을 자동으로 검증합니다.*

---

## 💡 사용 방법 및 예제 (Usage Example)

프로젝트 코드 내에서 `Integer` 클래스를 활용하는 기본적인 코드 형태 및 연계 컴파일 방법입니다.

### `main.cpp` 예제 코드
```cpp
#include <iostream>
#include "bigint.hpp"

int main() {
    // 1. 다양한 생성자
    Integer zero;                              // 기본 생성자 (0으로 초기화)
    Integer a(123456789ULL);                   // 64비트 정수로 초기화
    Integer b(987654321ULL);                   // 64비트 정수로 초기화
    Integer big("999999999999999999999999999"); // 문자열로 초기화

    // 2. 산술 연산자
    Integer sum  = a + b;       // 덧셈
    Integer diff = b - a;       // 뺄셈
    Integer prod = a * b;       // 곱셈 (Karatsuba 알고리즘)
    Integer quot = b / a;       // 나눗셈
    Integer rem  = b % a;       // 나머지

    // 3. 복합 대입 연산자
    Integer c(100);
    c += a;   // c = c + a
    c -= b;   // c = c - b

    // 4. 비트 연산
    Integer shifted_l = a << 64;   // 좌측 시프트
    Integer shifted_r = a >> 10;   // 우측 시프트
    Integer inverted  = ~a;        // 비트 반전 (NOT)

    // 5. 부호 관련
    Integer neg = -a;              // 단항 마이너스 (부호 반전)
    Integer pos = Integer::abs(neg); // 절댓값
    Integer::negate(pos);          // 부호 변경 (in-place)

    // 6. 비교 연산
    bool eq = (a == b);   // 같은지 비교
    bool ne = (a != b);   // 다른지 비교
    bool lt = (a < b);    // 작은지 비교
    bool le = (a <= b);   // 작거나 같은지 비교

    // 7. 유틸리티
    std::string str = prod.to_string();         // 10진법 문자열 변환
    size_t words = prod.size();                  // Word(uint64_t) 개수
    int64_t msb = Integer::MSB(a);               // 최상위 비트 인덱스
    int64_t lsb = Integer::LSB(a);               // 최하위 비트 인덱스
    int64_t log = Integer::log2(a);              // ⌊log₂(a)⌋

    // 8. 스트림 입출력
    std::cout << "Sum = " << sum << std::endl;   // ostream 출력
    // std::cin >> a;                             // istream 입력

    return 0;
}
```

### 함께 컴파일하기
소스 코드를 직접 포함하여 빌드하거나, 정적 라이브러리를 생성한 후 링크하여 컴파일할 수 있습니다.

#### 소스 직접 컴파일
```bash
g++ -std=c++17 -Iinclude main.cpp src/bigint.cpp -o my_app
./my_app
```

#### 정적 라이브러리 링크 컴파일
```bash
g++ -std=c++17 -Iinclude main.cpp -L. -lbigint -o my_app
./my_app
```

---

## ⚡ 주요 기능 및 성능 스펙 (Technical Specification)

### 생성자 (Constructors)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `Integer()` | $\mathcal{O}(1)$ | 기본 값 `0`으로 초기화합니다. |
| `Integer(uint64_t, bool)` | $\mathcal{O}(1)$ | 64비트 정수와 부호 플래그로 초기화합니다. |
| `Integer(const vector<uint64_t>&)` | $\mathcal{O}(N)$ | 64비트 단어열(Word Array)로 초기화합니다. |
| `Integer(const Integer&)` | $\mathcal{O}(N)$ | 복사 생성자입니다. (default) |
| `Integer(const Integer&, bool)` | $\mathcal{O}(N)$ | 복사 생성 시 부호를 지정합니다. |
| `Integer(Integer&&)` | $\mathcal{O}(1)$ | 이동 생성자입니다. |
| `Integer(Integer&&, bool)` | $\mathcal{O}(1)$ | 이동 생성 시 부호를 지정합니다. |
| `Integer(const string&)` | $\mathcal{O}(N^2)$ | 10진법 문자열을 파싱하여 초기화합니다. |
| `Integer(iterator, iterator, mask, bool)` | $\mathcal{O}(N)$ | 반복자 범위와 비트 마스크로 초기화합니다. |

### 산술 연산 (Arithmetic Operations)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `plus(const Integer&, bool)` | $\mathcal{O}(N)$ | 부호에 관계없이 절댓값 덧셈을 수행합니다. 캐리 무시 옵션을 제공합니다. |
| `minus(const Integer&)` | $\mathcal{O}(N)$ | 부호에 관계없이 절댓값 뺄셈을 수행합니다. 2의 보수법을 활용합니다. |
| `karatsuba_mul(const Integer&)` | $\mathcal{O}(N^{\log_2 3})$ | Karatsuba 알고리즘 기반 곱셈을 수행합니다. |
| `operator+(const Integer&)` | $\mathcal{O}(N)$ | 부호를 고려한 산술 덧셈입니다. |
| `operator+=(const Integer&)` | $\mathcal{O}(N)$ | 부호를 고려한 산술 덧셈 후 결과를 저장합니다. |
| `operator-(const Integer&)` | $\mathcal{O}(N)$ | 부호를 고려한 산술 뺄셈입니다. |
| `operator-=(const Integer&)` | $\mathcal{O}(N)$ | 부호를 고려한 산술 뺄셈 후 결과를 저장합니다. |
| `operator*(const Integer&)` | $\mathcal{O}(N^{\log_2 3})$ | Karatsuba 알고리즘 기반 곱셈 연산자입니다. |
| `operator/(const Integer&)` | $\mathcal{O}(N^{\log_2 3} \log N)$ | 나눗셈을 수행합니다. 0으로 나누면 예외를 던집니다. |
| `operator%(const Integer&)` | $\mathcal{O}(N^{\log_2 3} \log N)$ | 나머지 연산을 수행합니다. |
| `operator-()` *(단항)* | $\mathcal{O}(N)$ | 부호를 반전한 새로운 객체를 반환합니다. |

### 비트 연산 (Bitwise Operations)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `operator~()` | $\mathcal{O}(N)$ | 모든 비트를 반전(NOT)합니다. |
| `operator<<(uint64_t)` | $\mathcal{O}(N + k)$ | 좌측 비트 시프트를 수행합니다. |
| `operator>>(uint64_t)` | $\mathcal{O}(N)$ | 우측 비트 시프트를 수행합니다. |
| `operator<<=(uint64_t)` | $\mathcal{O}(N + k)$ | 좌측 비트 시프트 후 결과를 저장합니다. |
| `operator>>=(uint64_t)` | $\mathcal{O}(N)$ | 우측 비트 시프트 후 결과를 저장합니다. |
| `operator[](pair<uint64_t, uint64_t>)` | $\mathcal{O}(N)$ | 특정 비트 구간을 추출합니다. |

### 비교 연산 (Comparison Operations)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `operator==(const Integer&)` | $\mathcal{O}(N)$ | 두 정수가 같은지 비교합니다. |
| `operator!=(const Integer&)` | $\mathcal{O}(N)$ | 두 정수가 다른지 비교합니다. |
| `operator<(const Integer&)` | $\mathcal{O}(N)$ | 작은지 비교합니다. |
| `operator<=(const Integer&)` | $\mathcal{O}(N)$ | 작거나 같은지 비교합니다. |

### 대입 연산 (Assignment Operations)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `operator=(const Integer&)` | $\mathcal{O}(N)$ | 복사 대입 연산자입니다. |
| `operator=(Integer&&)` | $\mathcal{O}(1)$ | 이동 대입 연산자입니다. |
| `operator=(const string&)` | $\mathcal{O}(N^2)$ | 문자열을 파싱하여 대입합니다. |

### 정적 메서드 (Static Methods)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `Integer::abs(Integer&)` | $\mathcal{O}(1)$ | lvalue 참조의 절댓값을 취합니다. (in-place) |
| `Integer::abs(Integer&&)` | $\mathcal{O}(1)$ | rvalue 참조의 절댓값을 취합니다. |
| `Integer::abs(const Integer&)` | $\mathcal{O}(N)$ | const 참조의 절댓값 복사본을 반환합니다. |
| `Integer::negate(Integer&)` | $\mathcal{O}(1)$ | lvalue 참조의 부호를 반전합니다. (in-place) |
| `Integer::negate(Integer&&)` | $\mathcal{O}(1)$ | rvalue 참조의 부호를 반전합니다. |
| `Integer::log2(const Integer&)` | $\mathcal{O}(N)$ | $\lfloor \log_2 N \rfloor$ 을 계산합니다. |
| `Integer::MSB(const Integer&)` | $\mathcal{O}(N)$ | 최상위 비트(MSB)의 인덱스를 반환합니다. |
| `Integer::LSB(const Integer&)` | $\mathcal{O}(N)$ | 최하위 비트(LSB)의 인덱스를 반환합니다. |
| `Integer::mod(const Integer&, uint64_t l)` | $\mathcal{O}(l)$ | $n \bmod 2^l$ 을 계산합니다. |
| `Integer::inverse_of(const Integer&, uint64_t l)` | $\mathcal{O}(M(l))$ | $\bmod 2^l$ 에 대한 역원을 구합니다. (홀수만 가능) |
| `Integer::reverse(const Integer&, uint64_t l)` | $\mathcal{O}(l)$ | $l$개의 비트를 뒤집습니다. |

### 변환 및 유틸리티 (Conversion & Utility)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `to_string()` | $\mathcal{O}(N^2)$ | 10진법 문자열로 변환합니다. (Double Dabble 알고리즘) |
| `size()` | $\mathcal{O}(1)$ | 내부 Word 배열의 크기를 반환합니다. |
| `normalize()` | $\mathcal{O}(N)$ | 상위 0-값 Word를 제거하여 내부 표현을 정규화합니다. |
| `operator<<(ostream&, const Integer&)` | $\mathcal{O}(N^2)$ | 스트림 출력 연산자입니다. |
| `operator>>(istream&, Integer&)` | $\mathcal{O}(N^2)$ | 스트림 입력 연산자입니다. |

### 자유 함수 (Free Functions)

| API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `mod_add(a, b, m)` | $\mathcal{O}(N)$ | $(a + b) \bmod m$ 을 계산합니다. ($0 \le a, b < m$) |
| `redc(T, N, NN, R)` | $\mathcal{O}(M(R))$ | Montgomery Reduction: $T \cdot 2^{-R} \bmod N$ 을 계산합니다. |
| `mod_exp2(r, N, NN, R, twoR_mod_N)` | $\mathcal{O}(M(R) \log r)$ | Montgomery 형태의 모듈러 거듭제곱: $2^r \cdot 2^R \bmod N$ 을 계산합니다. |
| `mod_odd(n, m)` | $\mathcal{O}(N^{\log_2 3} \log N)$ | 홀수 $m$에 대한 모듈러 연산: $n \bmod m$ 을 계산합니다. |

*여기서 $N$은 정수를 구성하는 64비트 단어(Word)의 개수이며, $M(N)$은 곱셈 알고리즘의 시간 복잡도입니다.*

---

## 📄 라이선스 (License)

본 라이브러리는 [Apache License 2.0](LICENSE)에 따라 배포 및 수정이 자유롭습니다.
```text
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
```
