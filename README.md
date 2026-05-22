# 🔢 BigInteger C++ Library

[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=flat-square&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg?style=flat-square)](LICENSE)

임의 정밀도(Arbitrary-precision) 정수 연산을 지원하는 C++17 기반의 대용량 정수(BigInteger) 처리 라이브러리입니다. `uint64_t` 배열(Word 단위)을 활용하여 물리적인 메모리 한계까지 확장 가능한 정수를 표현하고 안정적인 산술 연산을 제공합니다.

---

## 📂 프로젝트 구조

```text
.
├── include/
│   └── bigint.hpp      # 라이브러리 헤더 파일 (Integer 클래스 정의)
├── src/
│   └── bigint.cpp      # 라이브러리 소스 코드 (핵심 연산 및 변환 구현)
├── tests/
│   └── test_bigint.cpp # 단위 테스트 소스 코드 (검증 및 시나리오 테스트)
└── LICENSE             # Apache 2.0 라이선스 파일
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

라이브러리의 기본 연산(덧셈, 뺄셈, 캐리 전파 등)에 대한 정밀성과 무결성을 검증하기 위한 단위 테스트 환경을 제공합니다.

#### 테스트 컴파일 및 링크
```bash
g++ -std=c++17 -Iinclude src/bigint.cpp tests/test_bigint.cpp -o test_runner
```

#### 테스트 러너 실행
```bash
./test_runner
```

#### 테스트 출력 예시
실행 시 아래와 같이 컬러풀하고 직관적인 테스트 통과 여부 요약 리포트가 터미널에 출력됩니다.

```text
===========================================
=== BigInteger Unit Tests: plus & minus ===
===========================================

--- Running plus() Tests ---
[ PASS ] plus: 0 + 0
[ PASS ] plus: 123 + 456 (Basic addition)
[ PASS ] plus: 0xFFFFFFFFFFFFFFFF + 1 (Carry propagation)
[ PASS ] plus: 0xFFFFFFFFFFFFFFFF + 0xFFFFFFFFFFFFFFFF (Max word values)

--- Running minus() Tests ---
[ FAIL ] minus: 456 - 123 (Basic subtraction)
         Expected: "333"
...
```

---

## 💡 사용 방법 및 예제 (Usage Example)

프로젝트 코드 내에서 `Integer` 클래스를 활용하는 기본적인 코드 형태 및 연계 컴파일 방법입니다.

### `main.cpp` 예제 코드
```cpp
#include <iostream>
#include "bigint.hpp"

int main() {
    // 1. 기본 생성자 (0으로 초기화)
    Integer zero;

    // 2. 64비트 정수를 이용한 초기화
    Integer a(123456789ULL);
    Integer b(987654321ULL);

    // 3. 덧셈 연산 (plus)
    Integer sum = a.plus(b);
    std::cout << "Sum (123456789 + 987654321) = " << sum.to_string() << std::endl;

    // 4. 뺄셈 연산 (minus)
    Integer diff = b.minus(a);
    std::cout << "Diff (987654321 - 123456789) = " << diff.to_string() << std::endl;

    // 5. 비트 반전 연산 (~ operator)
    Integer inverted = ~a;
    std::cout << "Inverted Bits of a = " << inverted.to_string() << std::endl;

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

| 기능 / API | 시간 복잡도 | 세부 설명 |
| :--- | :--- | :--- |
| `Integer()` | $\mathcal{O}(1)$ | 기본 값 `0`으로 대용량 정수를 초기화합니다. |
| `Integer(uint64_t)` | $\mathcal{O}(1)$ | 단일 64비트 정수를 매개변수로 받아 초기화합니다. |
| `Integer(std::vector<uint64_t>)` | $\mathcal{O}(N)$ | 임의의 크기를 가진 64비트 단어열(Word Array) 구조로 초기화합니다. |
| `plus(const Integer&)` | $\mathcal{O}(N)$ | 두 수의 산술 합을 올바르게 누적 계산합니다. 단어 간 캐리(Carry) 발생 시 자동으로 자릿수를 확장합니다. |
| `minus(const Integer&)` | $\mathcal{O}(N)$ | 2의 보수법(2's Complement)을 활용하여 내부적으로 보수 계산 후 가산 연산을 수행해 차(Difference)를 구합니다. |
| `operator~()` | $\mathcal{O}(N)$ | 모든 단어의 비트를 NOT 연산하여 반전시킵니다. |
| `to_string()` | $\mathcal{O}(N)$ | 대용량 정수를 10진수 문자열로 포맷팅하여 반환합니다. 단어 경계에서의 자릿수 유실을 막기 위해 16자리 패딩 기능을 포함합니다. |

*여기서 $N$은 정수를 구성하는 64비트 단어(Word)의 갯수입니다.*

---

## 📄 라이선스 (License)

본 라이브러리는 [Apache License 2.0](LICENSE)에 따라 배포 및 수정이 자유롭습니다.
```text
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0
```
