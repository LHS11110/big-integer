import unittest
import subprocess
import os
import random
import time

# Path to the compiled C++ test runner
RUNNER_PATH = "./test_runner"

def to_words_str(py_int: int):
    is_neg: bool = py_int < 0
    abs_val: int = abs(py_int)
    words: list[int] = []
    if abs_val == 0:
        words.append(0)
    else:
        while abs_val > 0:
            words.append(abs_val & 0xFFFFFFFFFFFFFFFF)
            abs_val >>= 64
    return "-" if is_neg else "+", ",".join(str(w) for w in words)

def parse_cpp_output(output_str: str):
    output_str = output_str.strip()
    if output_str.startswith("ERROR"):
        return output_str
    if not output_str:
        return 0
    return int(output_str)

def run_cpp(method: str, *args: int | str):
    cmd: list[str] = [RUNNER_PATH, method] + [str(a) for a in args]
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, check=True)
    return res.stdout.strip()

def generate_random_digits(length: int) -> int:
    if length <= 0:
        return 0
    first_digit = random.randint(1, 9)
    rest_digits = "".join(str(random.randint(0, 9)) for _ in range(length - 1))
    return int(str(first_digit) + rest_digits)

def generate_random_signed(length: int) -> int:
    val = generate_random_digits(length)
    return -val if random.choice([True, False]) else val

def generate_random_odd(length: int) -> int:
    """100~200자리의 양의 홀수를 생성합니다."""
    val = generate_random_digits(length)
    if val % 2 == 0:
        val += 1
    return val

def bit_length(n: int) -> int:
    """양의 정수 n의 비트 길이를 반환합니다 (⌊log2(n)⌋)."""
    if n <= 0:
        return -1
    return n.bit_length() - 1

def lsb(n: int) -> int:
    """양의 정수 n의 LSB 인덱스를 반환합니다."""
    if n == 0:
        return -1
    return (n & -n).bit_length() - 1

def reverse_bits(n: int, l: int) -> int:
    """n의 하위 l비트를 뒤집습니다."""
    result = 0
    for i in range(l):
        if n & (1 << i):
            result |= 1 << (l - 1 - i)
    return result


# ─────────────────────────────────────────────────────────────
# 테스트 반복 횟수 (각 테스트 메서드에서 사용)
# ─────────────────────────────────────────────────────────────
ITER = 100


class TestBigIntPublicAPI(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Compile the C++ test runner
        compile_cmd = ["g++", "-std=c++17", "-O2", "-Iinclude", "src/bigint.cpp", "tests/test_runner.cpp", "-o", "test_runner"]
        subprocess.run(compile_cmd, check=True)

    @classmethod
    def tearDownClass(cls):
        if os.path.exists(RUNNER_PATH):
            try:
                os.remove(RUNNER_PATH)
            except OSError:
                pass

    def setUp(self):
        self._start_time = time.time()

    def tearDown(self):
        duration = time.time() - self._start_time
        print(f"\n[TIME] {self.id().split('.')[-1]}: {duration:.4f}s", flush=True)

    # ═══════════════════════════════════════════════════════════
    #  생성자 (Constructors)
    # ═══════════════════════════════════════════════════════════

    def test_construct_default(self):
        """기본 생성자: Integer() == 0"""
        for _ in range(ITER):
            out = run_cpp("construct_default")
            self.assertEqual(parse_cpp_output(out), 0)

    def test_construct_uint64(self):
        """uint64_t 생성자: Integer(val, is_neg)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            val = abs(a) & 0xFFFFFFFFFFFFFFFF
            is_neg = a < 0
            out = run_cpp("construct_uint64", val, "true" if is_neg else "false")
            expected = -val if is_neg else val
            self.assertEqual(parse_cpp_output(out), expected)

    def test_construct_vector(self):
        """vector<uint64_t> 생성자: Integer(words)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            abs_val = abs(a)
            _, words_str = to_words_str(abs_val)
            out = run_cpp("construct_vector", words_str)
            self.assertEqual(parse_cpp_output(out), abs_val)

    def test_construct_copy(self):
        """복사 생성자: Integer(src, new_neg)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            new_neg = random.choice([True, False])
            out = run_cpp("construct_copy", str(a), "true" if new_neg else "false")
            expected = -abs(a) if new_neg else abs(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_construct_move(self):
        """이동 생성자: Integer(std::move(src))"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("construct_move", str(a))
            self.assertEqual(parse_cpp_output(out), a)

    def test_construct_move_sign(self):
        """이동 생성자(부호 지정): Integer(std::move(src), new_neg)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            new_neg = random.choice([True, False])
            out = run_cpp("construct_move_sign", str(a), "true" if new_neg else "false")
            expected = -abs(a) if new_neg else abs(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_construct_string(self):
        """문자열 생성자: Integer(str)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("construct_string", str(a))
            self.assertEqual(parse_cpp_output(out), a)

    # ═══════════════════════════════════════════════════════════
    #  정적 메서드 (Static methods)
    # ═══════════════════════════════════════════════════════════

    def test_negate_lvalue(self):
        """Integer::negate(lvalue) — 부호 반전"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("negate_lvalue", str(a))
            self.assertEqual(parse_cpp_output(out), -a)

    def test_negate_rvalue(self):
        """Integer::negate(rvalue) — 부호 반전"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("negate_rvalue", str(a))
            self.assertEqual(parse_cpp_output(out), -a)

    def test_abs_lvalue(self):
        """Integer::abs(lvalue) — 절댓값"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("abs_lvalue", str(a))
            self.assertEqual(parse_cpp_output(out), abs(a))

    def test_abs_rvalue(self):
        """Integer::abs(rvalue) — 절댓값"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("abs_rvalue", str(a))
            self.assertEqual(parse_cpp_output(out), abs(a))

    def test_log2(self):
        """Integer::log2(n) — ⌊log₂(n)⌋"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            out = run_cpp("log2", str(a))
            expected = bit_length(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_log2_zero(self):
        """Integer::log2(0) == -1"""
        out = run_cpp("log2", "0")
        self.assertEqual(parse_cpp_output(out), -1)

    def test_msb(self):
        """Integer::MSB(n) — 최상위 비트 인덱스 (== log2)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            out = run_cpp("msb", str(a))
            expected = bit_length(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_lsb(self):
        """Integer::LSB(n) — 최하위 비트 인덱스"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            out = run_cpp("lsb", str(a))
            expected = lsb(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_lsb_zero(self):
        """Integer::LSB(0) == -1"""
        out = run_cpp("lsb", "0")
        self.assertEqual(parse_cpp_output(out), -1)

    # ═══════════════════════════════════════════════════════════
    #  부호 비관련 산술 (plus / minus)
    # ═══════════════════════════════════════════════════════════

    def test_plus(self):
        """plus(rhs, ignore_carry)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)

            # 1) ignore_carry = False
            out = run_cpp("plus", str(a), str(b), "false")
            self.assertEqual(parse_cpp_output(out), a + b)

            # 2) ignore_carry = True
            out = run_cpp("plus", str(a), str(b), "true")
            _, w1 = to_words_str(a)
            _, w2 = to_words_str(b)
            len_w1 = len(w1.split(',')) if w1 else 1
            len_w2 = len(w2.split(',')) if w2 else 1
            limit = 1 << (64 * max(len_w1, len_w2))
            self.assertEqual(parse_cpp_output(out), (a + b) % limit)

    def test_minus(self):
        """minus(rhs) — 부호 무시 뺄셈"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)
            out = run_cpp("minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b)

    # ═══════════════════════════════════════════════════════════
    #  단항 연산자 (Unary Operators)
    # ═══════════════════════════════════════════════════════════

    def test_operator_negate(self):
        """operator-() — 부호 반전"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_negate", str(a))
            self.assertEqual(parse_cpp_output(out), -a)

    def test_operator_tilde(self):
        """operator~() — 비트 NOT (내부 배열 반전)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            out = run_cpp("operator_tilde", str(a))
            _, words = to_words_str(a)
            words_list = [int(w) for w in words.split(",")]
            expected_words = [w ^ 0xFFFFFFFFFFFFFFFF for w in words_list]
            expected_val = 0
            for i, w in enumerate(expected_words):
                expected_val |= w << (64 * i)
            self.assertEqual(parse_cpp_output(out), expected_val)

    # ═══════════════════════════════════════════════════════════
    #  시프트 연산 (Shift Operators)
    # ═══════════════════════════════════════════════════════════

    def test_operator_lshift(self):
        """operator<<(shift) — 좌측 비트 시프트"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_lshift", str(a), shift)
            self.assertEqual(parse_cpp_output(out), a << shift)

    def test_operator_rshift(self):
        """operator>>(shift) — 우측 비트 시프트"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_rshift", str(a), shift)
            abs_shifted = abs(a) >> shift
            expected = -abs_shifted if a < 0 else abs_shifted
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_lshift_assign(self):
        """operator<<=(shift) — 좌측 비트 시프트 대입"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_lshift_assign", str(a), shift)
            self.assertEqual(parse_cpp_output(out), a << shift)

    def test_operator_rshift_assign(self):
        """operator>>=(shift) — 우측 비트 시프트 대입"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_rshift_assign", str(a), shift)
            abs_shifted = abs(a) >> shift
            expected = -abs_shifted if a < 0 else abs_shifted
            self.assertEqual(parse_cpp_output(out), expected)

    def test_lshift_zero(self):
        """shift == 0일 때 원본 유지"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_lshift", str(a), 0)
            self.assertEqual(parse_cpp_output(out), a)

    def test_rshift_large(self):
        """shift가 비트 길이보다 큰 경우 0 반환"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            shift = random.randint(1000, 2000)
            out = run_cpp("operator_rshift", str(a), shift)
            expected = a >> shift
            self.assertEqual(parse_cpp_output(out), expected)

    # ═══════════════════════════════════════════════════════════
    #  산술 연산자 (Arithmetic Operators)
    # ═══════════════════════════════════════════════════════════

    def test_operator_plus(self):
        """operator+(rhs) — 산술 덧셈 (부호 고려)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_plus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b)

    def test_operator_plus_assign(self):
        """operator+=(rhs) — 산술 덧셈 대입"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_plus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b)

    def test_operator_minus(self):
        """operator-(rhs) — 산술 뺄셈 (부호 고려)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b)

    def test_operator_minus_assign(self):
        """operator-=(rhs) — 산술 뺄셈 대입"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_minus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b)

    def test_operator_mul(self):
        """operator*(rhs) — 곱셈 (karatsuba 기반)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_mul", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a * b)

    def test_operator_mul_zero(self):
        """0과의 곱셈"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_mul", str(a), "0")
            self.assertEqual(parse_cpp_output(out), 0)
            out = run_cpp("operator_mul", "0", str(a))
            self.assertEqual(parse_cpp_output(out), 0)

    def test_karatsuba_mul(self):
        """karatsuba_mul(rhs) — 카라추바 곱셈"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("karatsuba_mul", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a * b)

    def test_operator_div(self):
        """operator/(rhs) — 나눗셈 (정수 몫)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(50, 100)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)
            out = run_cpp("operator_div", str(a), str(b))
            expected = a // b
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_div_same_magnitude(self):
        """동일 자릿수의 나눗셈"""
        for _ in range(ITER):
            length = random.randint(100, 200)
            a = generate_random_digits(length)
            b = generate_random_digits(length)
            if b == 0:
                b = 1
            out = run_cpp("operator_div", str(a), str(b))
            expected = a // b
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_div_smaller_dividend(self):
        """피제수 < 제수 → 몫 = 0"""
        for _ in range(ITER):
            len_a = random.randint(100, 150)
            len_b = random.randint(160, 200)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)
            out = run_cpp("operator_div", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), 0)

    def test_operator_div_by_zero(self):
        """0으로 나누기 → ERROR"""
        len_a = random.randint(100, 200)
        a = generate_random_digits(len_a)
        out = run_cpp("operator_div", str(a), "0")
        self.assertTrue(out.startswith("ERROR"))

    def test_operator_div_signed(self):
        """operator/(rhs) — 부호 있는 나눗셈 (모든 부호 조합)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(50, 100)
            a_abs = generate_random_digits(len_a)
            b_abs = generate_random_digits(len_b)

            # (+a) / (+b)
            out = run_cpp("operator_div", str(a_abs), str(b_abs))
            self.assertEqual(parse_cpp_output(out), a_abs // b_abs)

            # (+a) / (-b)
            out = run_cpp("operator_div", str(a_abs), str(-b_abs))
            expected = -(a_abs // b_abs) - (1 if a_abs % b_abs != 0 else 0)
            self.assertEqual(parse_cpp_output(out), expected)

            # (-a) / (+b)
            out = run_cpp("operator_div", str(-a_abs), str(b_abs))
            expected = -(a_abs // b_abs) - (1 if a_abs % b_abs != 0 else 0)
            self.assertEqual(parse_cpp_output(out), expected)

            # (-a) / (-b)
            out = run_cpp("operator_div", str(-a_abs), str(-b_abs))
            self.assertEqual(parse_cpp_output(out), a_abs // b_abs)

    def test_operator_div_signed_smaller_dividend(self):
        """음수 나눗셈: |a| < |b| 인 경우"""
        for _ in range(ITER):
            len_a = random.randint(100, 150)
            len_b = random.randint(160, 200)
            a_abs = generate_random_digits(len_a)
            b_abs = generate_random_digits(len_b)

            # (+a) / (-b) where |a| < |b| → -1 (나머지가 있으므로)
            out = run_cpp("operator_div", str(a_abs), str(-b_abs))
            self.assertEqual(parse_cpp_output(out), -1)

            # (-a) / (+b) where |a| < |b| → -1
            out = run_cpp("operator_div", str(-a_abs), str(b_abs))
            self.assertEqual(parse_cpp_output(out), -1)

            # (-a) / (-b) where |a| < |b| → 0
            out = run_cpp("operator_div", str(-a_abs), str(-b_abs))
            self.assertEqual(parse_cpp_output(out), 0)

    def test_operator_div_signed_equal_magnitude(self):
        """음수 나눗셈: |a| == |b| 인 경우"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a_abs = generate_random_digits(len_a)

            # (+a) / (-a) → -1
            out = run_cpp("operator_div", str(a_abs), str(-a_abs))
            self.assertEqual(parse_cpp_output(out), -1)

            # (-a) / (+a) → -1
            out = run_cpp("operator_div", str(-a_abs), str(a_abs))
            self.assertEqual(parse_cpp_output(out), -1)

            # (-a) / (-a) → 1
            out = run_cpp("operator_div", str(-a_abs), str(-a_abs))
            self.assertEqual(parse_cpp_output(out), 1)

    def test_operator_div_signed_exact(self):
        """음수 나눗셈: 나머지가 0인 경우 (a = q * b)"""
        for _ in range(ITER):
            len_q = random.randint(50, 100)
            len_b = random.randint(50, 100)
            q = generate_random_digits(len_q)
            b_abs = generate_random_digits(len_b)
            a_abs = q * b_abs  # 정확히 나누어 떨어짐

            # (+a) / (-b) → -q
            out = run_cpp("operator_div", str(a_abs), str(-b_abs))
            self.assertEqual(parse_cpp_output(out), -q)

            # (-a) / (+b) → -q
            out = run_cpp("operator_div", str(-a_abs), str(b_abs))
            self.assertEqual(parse_cpp_output(out), -q)

            # (-a) / (-b) → +q
            out = run_cpp("operator_div", str(-a_abs), str(-b_abs))
            self.assertEqual(parse_cpp_output(out), q)

    # ═══════════════════════════════════════════════════════════
    #  나머지 연산자 (Modulo Operator %)
    # ═══════════════════════════════════════════════════════════

    def test_operator_mod(self):
        """operator%(rhs) — 나머지 (양수 % 양수)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(50, 100)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)
            out = run_cpp("operator_mod", str(a), str(b))
            expected = a % b
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_mod_same_magnitude(self):
        """동일 자릿수의 나머지"""
        for _ in range(ITER):
            length = random.randint(100, 200)
            a = generate_random_digits(length)
            b = generate_random_digits(length)
            if b == 0:
                b = 1
            out = run_cpp("operator_mod", str(a), str(b))
            expected = a % b
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_mod_smaller_dividend(self):
        """피제수 < 제수 → 나머지 = 피제수"""
        for _ in range(ITER):
            len_a = random.randint(100, 150)
            len_b = random.randint(160, 200)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)
            out = run_cpp("operator_mod", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a)

    def test_operator_mod_by_zero(self):
        """0으로 나머지 → ERROR"""
        len_a = random.randint(100, 200)
        a = generate_random_digits(len_a)
        out = run_cpp("operator_mod", str(a), "0")
        self.assertTrue(out.startswith("ERROR"))

    def test_operator_mod_signed(self):
        """operator%(rhs) — 부호 있는 나머지 (모든 부호 조합, Python semantics)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(50, 100)
            a_abs = generate_random_digits(len_a)
            b_abs = generate_random_digits(len_b)

            # (+a) % (+b)
            out = run_cpp("operator_mod", str(a_abs), str(b_abs))
            expected = a_abs % b_abs
            self.assertEqual(parse_cpp_output(out), expected)

            # (+a) % (-b)
            out = run_cpp("operator_mod", str(a_abs), str(-b_abs))
            expected = a_abs % (-b_abs)
            self.assertEqual(parse_cpp_output(out), expected)

            # (-a) % (+b)
            out = run_cpp("operator_mod", str(-a_abs), str(b_abs))
            expected = (-a_abs) % b_abs
            self.assertEqual(parse_cpp_output(out), expected)

            # (-a) % (-b)
            out = run_cpp("operator_mod", str(-a_abs), str(-b_abs))
            expected = (-a_abs) % (-b_abs)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_mod_exact_division(self):
        """나머지가 0인 경우 (a = q * b)"""
        for _ in range(ITER):
            len_q = random.randint(50, 100)
            len_b = random.randint(50, 100)
            q = generate_random_digits(len_q)
            b = generate_random_digits(len_b)
            a = q * b  # 정확히 나누어 떨어짐
            out = run_cpp("operator_mod", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), 0)

    def test_operator_mod_self(self):
        """a % a == 0"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            out = run_cpp("operator_mod", str(a), str(a))
            self.assertEqual(parse_cpp_output(out), 0)

    def test_operator_mod_one(self):
        """a % 1 == 0"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_mod", str(a), "1")
            self.assertEqual(parse_cpp_output(out), 0)

    def test_div_mod_consistency(self):
        """(a / b) * b + (a % b) == a (모든 부호 조합)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(50, 100)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)

            out_div = run_cpp("operator_div", str(a), str(b))
            q = parse_cpp_output(out_div)

            out_mod = run_cpp("operator_mod", str(a), str(b))
            r = parse_cpp_output(out_mod)

            # q * b + r == a 가 성립해야 함
            self.assertEqual(q * b + r, a,
                             f"Failed: ({a}) / ({b}) = {q}, ({a}) % ({b}) = {r}, "
                             f"q*b+r = {q * b + r} != {a}")

    # ═══════════════════════════════════════════════════════════
    #  비교 연산자 (Comparison Operators)
    # ═══════════════════════════════════════════════════════════

    def test_operator_le(self):
        """operator<=(rhs)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_le", str(a), str(b))
            expected = "true" if a <= b else "false"
            self.assertEqual(out, expected)

    def test_operator_lt(self):
        """operator<(rhs)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_lt", str(a), str(b))
            expected = "true" if a < b else "false"
            self.assertEqual(out, expected)

    def test_operator_ne(self):
        """operator!=(rhs)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_ne", str(a), str(b))
            expected = "true" if a != b else "false"
            self.assertEqual(out, expected)

    def test_operator_eq(self):
        """operator==(rhs)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_eq", str(a), str(b))
            expected = "true" if a == b else "false"
            self.assertEqual(out, expected)

    def test_comparison_equal_values(self):
        """동일한 값으로 비교 연산 테스트"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            # a == a
            out = run_cpp("operator_eq", str(a), str(a))
            self.assertEqual(out, "true")
            # a <= a
            out = run_cpp("operator_le", str(a), str(a))
            self.assertEqual(out, "true")
            # a < a → false
            out = run_cpp("operator_lt", str(a), str(a))
            self.assertEqual(out, "false")
            # a != a → false
            out = run_cpp("operator_ne", str(a), str(a))
            self.assertEqual(out, "false")

    # ═══════════════════════════════════════════════════════════
    #  대입 연산자 (Assignment Operators)
    # ═══════════════════════════════════════════════════════════

    def test_operator_assign_copy(self):
        """operator=(copy)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_assign_copy", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), b)

    def test_operator_assign_move(self):
        """operator=(move)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_assign_move", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), b)

    # ═══════════════════════════════════════════════════════════
    #  변환 / 출력 (Conversion / Output)
    # ═══════════════════════════════════════════════════════════

    def test_to_string(self):
        """to_string() — 10진법 문자열 변환"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("to_string", str(a))
            self.assertEqual(out, str(a))

    def test_ostream_out(self):
        """operator<< — 출력 스트림"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("ostream_out", str(a))
            self.assertEqual(out, str(a))

    def test_istream_in(self):
        """operator>> — 입력 스트림"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("istream_in", str(a))
            self.assertEqual(parse_cpp_output(out), a)

    def test_size(self):
        """size() — 내부 배열 크기"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("size", str(a))
            _, words = to_words_str(a)
            expected_size = len(words.split(",")) if words else 1
            self.assertEqual(parse_cpp_output(out), expected_size)

    # ═══════════════════════════════════════════════════════════
    #  모듈러 연산 (Modular Arithmetic)
    # ═══════════════════════════════════════════════════════════

    def test_mod(self):
        """Integer::mod(n, l) — n mod 2^l (양수)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            l = random.randint(100, 600)
            out = run_cpp("mod", str(a), l)
            expected = a % (1 << l)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_mod_negative(self):
        """Integer::mod(n, l) — 음수 n에 대한 mod 2^l"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            l = random.randint(100, 600)
            out = run_cpp("mod", str(-a), l)
            expected = (-a) % (1 << l)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_inverse_of(self):
        """Integer::inverse_of(n, l) — n^-1 mod 2^l (n은 홀수)"""
        for _ in range(ITER):
            length = random.randint(100, 200)
            n = generate_random_odd(length)
            l = random.randint(100, 400)
            out = run_cpp("inverse_of", str(n), l)
            inv = parse_cpp_output(out)
            # 검증: n * inv ≡ 1 (mod 2^l)
            mod_val = 1 << l
            self.assertEqual((n * inv) % mod_val, 1)

    def test_mod_add(self):
        """mod_add(a, b, m) — (a + b) mod m (a, b < m)"""
        for _ in range(ITER):
            len_m = random.randint(101, 200)
            m = generate_random_digits(len_m)

            # a < m, b < m이 되도록 생성
            a = generate_random_digits(random.randint(100, len_m - 1)) % m
            b = generate_random_digits(random.randint(100, len_m - 1)) % m
            if a == 0:
                a = 1
            if b == 0:
                b = 1

            out = run_cpp("mod_add", str(a), str(b), str(m))
            expected = (a + b) % m
            self.assertEqual(parse_cpp_output(out), expected)

    def test_mod_odd(self):
        """mod_odd(n, m) — n mod m (m은 홀수)"""
        for _ in range(ITER):
            len_n = random.randint(100, 200)
            len_m = random.randint(50, 100)
            n = generate_random_digits(len_n)
            m = generate_random_odd(len_m)
            out = run_cpp("mod_odd", str(n), str(m))
            expected = n % m
            self.assertEqual(parse_cpp_output(out), expected)

    # ═══════════════════════════════════════════════════════════
    #  비트 조작 (Bit Manipulation)
    # ═══════════════════════════════════════════════════════════

    def test_reverse(self):
        """Integer::reverse(n, l) — 하위 l비트 반전"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            bl = bit_length(a)
            l = bl + 1 if bl >= 0 else 1
            out = run_cpp("reverse", str(a), l)
            expected = reverse_bits(a, l)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_subscript(self):
        """operator[{first, second}] — 비트 구간 추출"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            total_bits = bit_length(a) + 1
            if total_bits < 2:
                continue

            # 유효한 구간 [first, second) 생성
            first = random.randint(0, total_bits // 2)
            second = random.randint(first + 1, min(total_bits, first + 200))

            out = run_cpp("operator_subscript", str(a), first, second)

            # 파이썬에서 기대값 계산: a[first:second]
            # mask = (1 << second) - 1 으로 하위 second 비트 마스킹 후 first만큼 시프트
            mask = (1 << second) - 1
            expected = (a & mask) >> first
            self.assertEqual(parse_cpp_output(out), expected)

    # ═══════════════════════════════════════════════════════════
    #  산술 에지 케이스 (Edge Cases)
    # ═══════════════════════════════════════════════════════════

    def test_add_subtract_identity(self):
        """a + b - b == a"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out_add = run_cpp("operator_plus", str(a), str(b))
            sum_val = parse_cpp_output(out_add)
            out_sub = run_cpp("operator_minus", str(sum_val), str(b))
            self.assertEqual(parse_cpp_output(out_sub), a)

    def test_mul_commutativity(self):
        """a * b == b * a"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out1 = run_cpp("operator_mul", str(a), str(b))
            out2 = run_cpp("operator_mul", str(b), str(a))
            self.assertEqual(parse_cpp_output(out1), parse_cpp_output(out2))
            self.assertEqual(parse_cpp_output(out1), a * b)

    def test_div_mul_consistency(self):
        """(a / b) * b + (a mod b) == a (b가 홀수일 때)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            len_b = random.randint(50, 100)
            a = generate_random_digits(len_a)
            b = generate_random_odd(len_b)

            out_div = run_cpp("operator_div", str(a), str(b))
            q = parse_cpp_output(out_div)

            out_mod = run_cpp("mod_odd", str(a), str(b))
            r = parse_cpp_output(out_mod)

            self.assertEqual(q * b + r, a)

    def test_shift_consistency(self):
        """(a << n) >> n == a (양수, shift < bit_length)"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_digits(len_a)
            shift = random.randint(0, 100)
            out_lshift = run_cpp("operator_lshift", str(a), shift)
            shifted = parse_cpp_output(out_lshift)
            out_rshift = run_cpp("operator_rshift", str(shifted), shift)
            self.assertEqual(parse_cpp_output(out_rshift), a)

    def test_negate_double(self):
        """-(-a) == a"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_negate", str(a))
            neg_a = parse_cpp_output(out)
            out2 = run_cpp("operator_negate", str(neg_a))
            self.assertEqual(parse_cpp_output(out2), a)

    def test_add_zero(self):
        """a + 0 == a"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_plus", str(a), "0")
            self.assertEqual(parse_cpp_output(out), a)

    def test_sub_self(self):
        """a - a == 0"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_minus", str(a), str(a))
            self.assertEqual(parse_cpp_output(out), 0)

    def test_mul_one(self):
        """a * 1 == a"""
        for _ in range(ITER):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_mul", str(a), "1")
            self.assertEqual(parse_cpp_output(out), a)


if __name__ == '__main__':
    unittest.main()
