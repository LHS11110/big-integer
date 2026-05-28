import unittest
import subprocess
import os
import random

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

class TestBigIntPublicAPI(unittest.TestCase):
    
    @classmethod
    def setUpClass(cls):
        # Compile the C++ test runner
        compile_cmd = ["g++", "-std=c++17", "-Iinclude", "src/bigint.cpp", "tests/test_runner.cpp", "-o", "test_runner"]
        subprocess.run(compile_cmd, check=True)

    @classmethod
    def tearDownClass(cls):
        if os.path.exists(RUNNER_PATH):
            try:
                os.remove(RUNNER_PATH)
            except OSError:
                pass

    def test_constructors(self):
        # 1. Default constructor Integer() -> Should initialize to 0
        out = run_cpp("construct_default")
        self.assertEqual(parse_cpp_output(out), 0)

        # 2. Integer(uint64_t, bool) constructor
        test_vals = [0, 1, 123456789, 0xFFFFFFFFFFFFFFFF]
        for val in test_vals:
            for is_neg in [False, True]:
                out = run_cpp("construct_uint64", val, "true" if is_neg else "false")
                expected = -val if is_neg else val
                self.assertEqual(parse_cpp_output(out), expected)

        # 3. Integer(std::vector<uint64_t>) constructor
        vector_cases: list[list[int]] = [
            [],
            [0],
            [12345],
            [0xFFFFFFFFFFFFFFFF, 0x123456789ABCDEF0],
            [0, 0, 0, 1]
        ]
        for arr in vector_cases:
            words_str: str = ",".join(str(w) for w in arr) if arr else "empty"
            out = run_cpp("construct_vector", words_str)
            val = 0
            for i, word in enumerate(arr):
                val |= word << (64 * i)
            self.assertEqual(parse_cpp_output(out), val)

        # 4. Copy constructor Integer(const Integer&, bool)
        src_val = 9876543210123456789
        out = run_cpp("construct_copy", str(src_val), "true")
        self.assertEqual(parse_cpp_output(out), -src_val)

        out = run_cpp("construct_copy", str(src_val), "false")
        self.assertEqual(parse_cpp_output(out), src_val)

        # 5. Move constructor Integer(Integer&&)
        src_val = -12345678901234567890987654321
        out = run_cpp("construct_move", str(src_val))
        self.assertEqual(parse_cpp_output(out), src_val)

        # 6. Move constructor with custom sign: Integer(Integer&&, const bool)
        src_val = 1234567890123456789
        out = run_cpp("construct_move_sign", str(src_val), "true")
        self.assertEqual(parse_cpp_output(out), -src_val)

        out = run_cpp("construct_move_sign", str(src_val), "false")
        self.assertEqual(parse_cpp_output(out), src_val)

    def test_string_constructor(self):
        # 1. Valid numbers
        valid_cases = [
            "0", "123", "-123", "+456", 
            "18446744073709551615", "-18446744073709551615",
            "9876543210987654321098765432109876543210",
            "-9876543210987654321098765432109876543210"
        ]
        for s in valid_cases:
            out = run_cpp("construct_string", s)
            expected = int(s)
            self.assertEqual(parse_cpp_output(out), expected, f"Failed for string constructor with: {s}")

        # 2. Invalid numbers (should throw std::invalid_argument)
        invalid_cases = ["abc", "12a3", "123-45", "--123", "   "]
        for s in invalid_cases:
            out = run_cpp("construct_string", s)
            self.assertTrue(out.startswith("ERROR"), f"Should fail for string: '{s}', instead got: '{out}'")

    def test_static_negate_methods(self):
        # static negate(Integer&)
        test_numbers = [0, 1, -1, 12345678901234567890, -98765432109876543210]
        for num in test_numbers:
            out = run_cpp("negate_lvalue", str(num))
            self.assertEqual(parse_cpp_output(out), -num)

        # static negate(Integer&&)
        for num in test_numbers:
            out = run_cpp("negate_rvalue", str(num))
            self.assertEqual(parse_cpp_output(out), -num)

    def test_plus_method(self):
        test_pairs = [
            (0, 0),
            (0, 12345),
            (12345, 0),
            (0xFFFFFFFFFFFFFFFF, 1),
            (0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF),
        ]
        # MASSIVE FUZZING: 1000 pairs of completely randomized bit sizes (1 to 1024 bits)
        for _ in range(1000):
            bits_a = random.randint(1, 1024)
            bits_b = random.randint(1, 1024)
            a = random.randint(0, 2**bits_a)
            b = random.randint(0, 2**bits_b)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            out = run_cpp("plus", str(a), str(b), "false")
            self.assertEqual(parse_cpp_output(out), a + b, f"Failed: plus({a}, {b}, false)")

            # ignore carry
            out = run_cpp("plus", str(a), str(b), "true")
            _, w1 = to_words_str(a)
            _, w2 = to_words_str(b)
            len_w1 = len(w1.split(',')) if w1 else 1
            len_w2 = len(w2.split(',')) if w2 else 1
            max_len = max(len_w1, len_w2)
            limit = 1 << (64 * max_len)
            expected = (a + b) % limit
            self.assertEqual(parse_cpp_output(out), expected, f"Failed: plus({a}, {b}, true)")

    def test_minus_method(self):
        test_pairs = [
            (0, 0),
            (100, 30),
            (30, 100),
            (0xFFFFFFFFFFFFFFFF, 1),
            (1, 0xFFFFFFFFFFFFFFFF),
        ]
        # MASSIVE FUZZING: 1000 pairs of completely randomized bit sizes (1 to 1024 bits)
        for _ in range(1000):
            bits_a = random.randint(1, 1024)
            bits_b = random.randint(1, 1024)
            a = random.randint(0, 2**bits_a)
            b = random.randint(0, 2**bits_b)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            out = run_cpp("minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b, f"Failed: minus({a}, {b})")

    def test_operator_negate(self):
        test_vals = [0, 1, -1, 1234567890123456789, -9876543210987654321]
        for val in test_vals:
            out = run_cpp("operator_negate", str(val))
            self.assertEqual(parse_cpp_output(out), -val)

    def test_operator_tilde(self):
        test_vals = [
            0, 1, 0xFFFFFFFFFFFFFFFF,
            0x123456789ABCDEF0,
            0x9876543210FEDCBA123456789ABCDEF0
        ]
        for val in test_vals:
            out = run_cpp("operator_tilde", str(val))
            _, words = to_words_str(val)
            words_list = [int(w) for w in words.split(",")]
            expected_words = [w ^ 0xFFFFFFFFFFFFFFFF for w in words_list]
            expected_val = 0
            for i, w in enumerate(expected_words):
                expected_val |= w << (64 * i)
            self.assertEqual(parse_cpp_output(out), expected_val)

    def test_operator_lshift_and_lshift_assign(self):
        test_vals = [
            0, 1, 0xFFFFFFFFFFFFFFFF,
            0x123456789ABCDEF0,
            0xABCDEF1234567890ABCDEF1234567890
        ]
        shifts = [0, 1, 5, 32, 63, 64, 65, 128]
        for val in test_vals:
            for shift in shifts:
                for is_neg in [False, True]:
                    num = -val if is_neg else val
                    
                    # << operator
                    out = run_cpp("operator_lshift", str(num), shift)
                    self.assertEqual(parse_cpp_output(out), num << shift)
                    
                    # <<= operator
                    out = run_cpp("operator_lshift_assign", str(num), shift)
                    self.assertEqual(parse_cpp_output(out), num << shift)

    def test_operator_rshift_and_rshift_assign(self):
        test_vals = [
            0, 1, 0xFFFFFFFFFFFFFFFF,
            0x123456789ABCDEF0,
            0xABCDEF1234567890ABCDEF1234567890
        ]
        shifts = [0, 1, 5, 32, 63, 64, 65, 128, 256]
        for val in test_vals:
            for shift in shifts:
                for is_neg in [False, True]:
                    num = -val if is_neg else val
                    
                    abs_shifted = abs(num) >> shift
                    expected = -abs_shifted if is_neg else abs_shifted
                    
                    # >> operator
                    out = run_cpp("operator_rshift", str(num), shift)
                    self.assertEqual(parse_cpp_output(out), expected, f"Failed: {num} >> {shift}")
                    
                    # >>= operator
                    out = run_cpp("operator_rshift_assign", str(num), shift)
                    self.assertEqual(parse_cpp_output(out), expected, f"Failed: {num} >>= {shift}")

    def test_operator_plus_and_plus_assign(self):
        test_pairs = [
            (0, 0), (12345, 67890), (-12345, 67890), (12345, -67890), (-12345, -67890),
            (0xFFFFFFFFFFFFFFFF, 1), (-0xFFFFFFFFFFFFFFFF, 1)
        ]
        # MASSIVE FUZZING: 1000 pairs of completely randomized signed integers (up to 1024 bits)
        for _ in range(1000):
            bits_a = random.randint(1, 1024)
            bits_b = random.randint(1, 1024)
            a = random.randint(-2**bits_a, 2**bits_a)
            b = random.randint(-2**bits_b, 2**bits_b)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            # operator+
            out = run_cpp("operator_plus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b, f"Failed: {a} + {b}")
            
            # operator+=
            out = run_cpp("operator_plus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b, f"Failed: {a} += {b}")

    def test_operator_minus_and_minus_assign(self):
        test_pairs = [
            (0, 0), (12345, 67890), (-12345, 67890), (12345, -67890), (-12345, -67890),
            (0xFFFFFFFFFFFFFFFF, 1), (-0xFFFFFFFFFFFFFFFF, 1)
        ]
        # MASSIVE FUZZING: 1000 pairs of completely randomized signed integers (up to 1024 bits)
        for _ in range(1000):
            bits_a = random.randint(1, 1024)
            bits_b = random.randint(1, 1024)
            a = random.randint(-2**bits_a, 2**bits_a)
            b = random.randint(-2**bits_b, 2**bits_b)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            # operator-
            out = run_cpp("operator_minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b, f"Failed: {a} - {b}")
            
            # operator-=
            out = run_cpp("operator_minus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b, f"Failed: {a} -= {b}")

    def test_operator_le_and_ne_and_eq(self):
        test_pairs = [
            (0, 0), (1, 0), (0, 1), (-1, 0), (0, -1),
            (12345, 12345), (12345, 12346), (-12345, -12346),
            (258, 513), (513, 258)
        ]
        # MASSIVE FUZZING: 1000 pairs of completely randomized signed integers (up to 1024 bits)
        for _ in range(1000):
            bits_a = random.randint(1, 1024)
            bits_b = random.randint(1, 1024)
            a = random.randint(-2**bits_a, 2**bits_a)
            b = random.randint(-2**bits_b, 2**bits_b)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            # operator<=
            out = run_cpp("operator_le", str(a), str(b))
            expected = "true" if a <= b else "false"
            self.assertEqual(out, expected, f"Failed: {a} <= {b}")
            
            # operator!=
            out = run_cpp("operator_ne", str(a), str(b))
            expected = "true" if a != b else "false"
            self.assertEqual(out, expected, f"Failed: {a} != {b}")

            # operator==
            out = run_cpp("operator_eq", str(a), str(b))
            expected = "true" if a == b else "false"
            self.assertEqual(out, expected, f"Failed: {a} == {b}")

    def test_operator_assignment(self):
        # 1. operator=(const Integer&) copy assignment
        a, b = 12345678901234567890, -98765432109876543210
        out = run_cpp("operator_assign_copy", str(a), str(b))
        self.assertEqual(parse_cpp_output(out), b)

        # 2. operator=(Integer&&) move assignment
        out = run_cpp("operator_assign_move", str(a), str(b))
        self.assertEqual(parse_cpp_output(out), b)

    def test_to_string_and_ostream(self):
        # 1. to_string
        test_numbers = [
            0, 1, -1, 10, -10, 123, -456, 
            18446744073709551615, -18446744073709551615,
            9876543210987654321098765432109876543210,
            -9876543210987654321098765432109876543210
        ]
        for num in test_numbers:
            # to_string()
            out = run_cpp("to_string", str(num))
            self.assertEqual(out, str(num), f"to_string failed for {num}")
            
            # operator<< (ostream output)
            out_ostream = run_cpp("ostream_out", str(num))
            self.assertEqual(out_ostream, str(num), f"ostream output failed for {num}")

    # ======================================================================
    # EXTRA EXTREME EDGE CASES AND STRESS TESTS
    # ======================================================================

    def test_extreme_large_string_and_to_string(self):
        # 스트레스 테스트: 100자리, 200자리, 300자리의 거대 문자열 파싱 및 to_string 역복원성 검증
        for length in [100, 200, 300]:
            for is_neg in [False, True]:
                # 무작위 초대형 정수 생성 (앞자리는 0이 아니도록 처리)
                digits = [str(random.randint(1, 9))] + [str(random.randint(0, 9)) for _ in range(length - 1)]
                str_val = "".join(digits)
                if is_neg:
                    str_val = "-" + str_val
                
                # C++ 문자열 생성자로 인스턴스를 빌드한 후 내부 상태 체크
                out_struct = run_cpp("construct_string", str_val)
                expected_int = int(str_val)
                self.assertEqual(parse_cpp_output(out_struct), expected_int, f"Failed parsing extreme large string of length {length}")
                
                # C++의 to_string()을 통해 반환된 문자열이 원본과 정확히 일치하는지 검증
                # Double Dabble 알고리즘의 극한 자릿수 안정성 확인
                out_str = run_cpp("to_string", str_val)
                self.assertEqual(out_str, str_val, f"Failed to_string reconstruction for extreme large number of length {length}")

    def test_extreme_shifts_and_normalization(self):
        # 시프트 대입 및 비대입 연산자들의 대량 시프트(Boundary, pop_back) 검증
        # 1. 256, 512, 1024비트의 극한의 쉬프트 테스트
        a = 0xDEADC0DEBAADF00D1234567890ABCDEF
        shifts = [256, 512, 1024]
        for shift in shifts:
            for is_neg in [False, True]:
                num = -a if is_neg else a
                
                # << 및 <<=
                out = run_cpp("operator_lshift", str(num), shift)
                self.assertEqual(parse_cpp_output(out), num << shift, f"Failed extreme lshift: {num} << {shift}")
                
                out_assign = run_cpp("operator_lshift_assign", str(num), shift)
                self.assertEqual(parse_cpp_output(out_assign), num << shift, f"Failed extreme lshift_assign: {num} <<= {shift}")
                
                # >> 및 >>=
                abs_shifted = abs(num) >> shift
                expected = -abs_shifted if is_neg else abs_shifted
                
                out_r = run_cpp("operator_rshift", str(num), shift)
                self.assertEqual(parse_cpp_output(out_r), expected, f"Failed extreme rshift: {num} >> {shift}")
                
                out_r_assign = run_cpp("operator_rshift_assign", str(num), shift)
                self.assertEqual(parse_cpp_output(out_r_assign), expected, f"Failed extreme rshift_assign: {num} >>= {shift}")

        # 2. 아주 작은 수에 대해 과도하게 큰 우측 시프트(>>=)를 적용해 0으로의 수렴 및 pop_back이 터지지 않는지 확인
        b = 123456
        for shift in [32, 64, 128]:
            for is_neg in [False, True]:
                num = -b if is_neg else b
                out = run_cpp("operator_rshift_assign", str(num), shift)
                # 절대치가 모두 수렴하여 0이 되고, 부호는 양수로 초기화되거나 음수 0이 정상 리셋되어 0이 되는지 확인
                self.assertEqual(parse_cpp_output(out), 0, f"Failed rshift-to-zero convergence: {num} >>= {shift}")

    def test_signed_arithmetic_combinations(self):
        # 사칙연산의 연속 부호 조합 시 is_negative 부호 유지 및 누출 검증
        test_pairs = [
            (2**128 - 1, -(2**128 - 1)), # 서로 절댓값이 같고 부호만 반대인 덧셈 -> 결과 0
            (-(2**128 - 1), 2**128 - 1),
            (2**192, -2**192),
            (-2**64, 2**64 - 1),
            (2**64 - 1, -2**64)
        ]
        
        for a, b in test_pairs:
            # (A) + (B)
            out = run_cpp("operator_plus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b, f"Failed combination: {a} + {b}")
            
            # (A) - (B)
            out = run_cpp("operator_minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b, f"Failed combination: {a} - {b}")

    def test_extreme_100_to_200_digits_random(self):
        # 100자리 이상 200자리 이하의 거대 랜덤 정수 대상 교차 검증 (최상위 자릿수 0 방지)
        for _ in range(100): # 100쌍 Fuzzing
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            # 10**(L-1) <= a < 10**L -> 최상위가 0이 아님을 보장
            a = random.randint(10**(len_a - 1), 10**len_a - 1)
            b = random.randint(10**(len_b - 1), 10**len_b - 1)
            
            # 랜덤 부호 부여
            a = -a if random.choice([True, False]) else a
            b = -b if random.choice([True, False]) else b
            
            # 1. String Constructor & to_string & ostream_out
            out_construct = run_cpp("construct_string", str(a))
            self.assertEqual(parse_cpp_output(out_construct), a, f"Failed construct_string for {a}")
            
            out_tostr = run_cpp("to_string", str(a))
            self.assertEqual(out_tostr, str(a), f"Failed to_string for {a}")
            
            out_ostream = run_cpp("ostream_out", str(a))
            self.assertEqual(out_ostream, str(a), f"Failed ostream_out for {a}")
            
            # 2. operator+ and operator+=
            out_plus = run_cpp("operator_plus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out_plus), a + b, f"Failed {a} + {b}")
            
            out_plus_assign = run_cpp("operator_plus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out_plus_assign), a + b, f"Failed {a} += {b}")
            
            # 3. operator- and operator-=
            out_minus = run_cpp("operator_minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out_minus), a - b, f"Failed {a} - {b}")
            
            out_minus_assign = run_cpp("operator_minus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out_minus_assign), a - b, f"Failed {a} -= {b}")
            
            # 4. operator- (negation)
            out_neg = run_cpp("operator_negate", str(a))
            self.assertEqual(parse_cpp_output(out_neg), -a, f"Failed -{a}")
            
            # 5. Comparisons (<=, !=, ==)
            expected_le = "true" if a <= b else "false"
            expected_ne = "true" if a != b else "false"
            expected_eq = "true" if a == b else "false"
            
            self.assertEqual(run_cpp("operator_le", str(a), str(b)), expected_le, f"Failed {a} <= {b}")
            self.assertEqual(run_cpp("operator_ne", str(a), str(b)), expected_ne, f"Failed {a} != {b}")
            self.assertEqual(run_cpp("operator_eq", str(a), str(b)), expected_eq, f"Failed {a} == {b}")
            
            # 6. Shifts (<<, <<=, >>, >>=)
            shift = random.randint(0, 200)
            
            # << & <<=
            out_lshift = run_cpp("operator_lshift", str(a), shift)
            self.assertEqual(parse_cpp_output(out_lshift), a << shift, f"Failed {a} << {shift}")
            
            out_lshift_assign = run_cpp("operator_lshift_assign", str(a), shift)
            self.assertEqual(parse_cpp_output(out_lshift_assign), a << shift, f"Failed {a} <<= {shift}")
            
            # >> & >>=
            abs_shifted = abs(a) >> shift
            expected_rshift = -abs_shifted if a < 0 else abs_shifted
            
            out_rshift = run_cpp("operator_rshift", str(a), shift)
            self.assertEqual(parse_cpp_output(out_rshift), expected_rshift, f"Failed {a} >> {shift}")
            
            out_rshift_assign = run_cpp("operator_rshift_assign", str(a), shift)
            self.assertEqual(parse_cpp_output(out_rshift_assign), expected_rshift, f"Failed {a} >>= {shift}")

if __name__ == '__main__':
    unittest.main()
