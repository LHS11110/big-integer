import unittest
import subprocess
import os
import random

# Path to the compiled C++ test runner
RUNNER_PATH = "./test_runner"

def to_words_str(py_int):
    is_neg = py_int < 0
    abs_val = abs(py_int)
    words = []
    if abs_val == 0:
        words.append(0)
    else:
        while abs_val > 0:
            words.append(abs_val & 0xFFFFFFFFFFFFFFFF)
            abs_val >>= 64
    return "-" if is_neg else "+", ",".join(str(w) for w in words)

def parse_cpp_output(output_str):
    parts = output_str.strip().split()
    if len(parts) == 0:
        return 0
    is_neg = parts[0] == "-"
    if len(parts) > 1:
        words = [int(w) for w in parts[1].split(",")]
    else:
        words = [0]
    
    val = 0
    for i, word in enumerate(words):
        val |= word << (64 * i)
    return -val if is_neg else val

def run_cpp(method, *args):
    cmd = [RUNNER_PATH, method] + [str(a) for a in args]
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
        # 1. Default constructor Integer() -> Should initialize to + 0
        out = run_cpp("construct_default")
        self.assertEqual(parse_cpp_output(out), 0)
        self.assertTrue(out.startswith("+"))

        # 2. Integer(uint64_t, bool) constructor
        test_vals = [0, 1, 123456789, 0xFFFFFFFFFFFFFFFF]
        for val in test_vals:
            for is_neg in [False, True]:
                out = run_cpp("construct_uint64", val, "true" if is_neg else "false")
                expected = -val if is_neg else val
                self.assertEqual(parse_cpp_output(out), expected)

        # 3. Integer(std::vector<uint64_t>) constructor
        vector_cases = [
            [],
            [0],
            [12345],
            [0xFFFFFFFFFFFFFFFF, 0x123456789ABCDEF0],
            [0, 0, 0, 1]
        ]
        for arr in vector_cases:
            words_str = ",".join(str(w) for w in arr) if arr else "empty"
            out = run_cpp("construct_vector", words_str)
            val = 0
            for i, word in enumerate(arr):
                val |= word << (64 * i)
            self.assertEqual(parse_cpp_output(out), val)

        # 4. Copy constructor Integer(const Integer&, bool)
        sign, words = to_words_str(9876543210123456789)
        out = run_cpp("construct_copy", sign, words, "true")
        self.assertEqual(parse_cpp_output(out), -9876543210123456789)

        out = run_cpp("construct_copy", sign, words, "false")
        self.assertEqual(parse_cpp_output(out), 9876543210123456789)

        # 5. Move constructor Integer(Integer&&)
        sign, words = to_words_str(-12345678901234567890987654321)
        out = run_cpp("construct_move", sign, words)
        self.assertEqual(parse_cpp_output(out), -12345678901234567890987654321)

        # 6. Move constructor with custom sign: Integer(Integer&&, const bool)
        sign, words = to_words_str(1234567890123456789)
        out = run_cpp("construct_move_sign", sign, words, "true")
        self.assertEqual(parse_cpp_output(out), -1234567890123456789)

        out = run_cpp("construct_move_sign", sign, words, "false")
        self.assertEqual(parse_cpp_output(out), 1234567890123456789)

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
        invalid_cases = ["abc", "12a3", "123-45", "--123", "", "   "]
        for s in invalid_cases:
            out = run_cpp("construct_string", s if s != "" else "empty")
            self.assertTrue(out.startswith("ERROR"), f"Should fail for string: '{s}', instead got: '{out}'")

    def test_mod10(self):
        # mod10 evaluates |left| % 10
        test_numbers = [0, 1, 9, 10, 11, 123456, 18446744073709551615, 98765432109876543210987654321]
        for num in test_numbers:
            for is_neg in [False, True]:
                val = -num if is_neg else num
                sign, words = to_words_str(val)
                out = run_cpp("mod10", sign, words)
                expected = abs(val) % 10
                self.assertEqual(int(out), expected, f"Failed: mod10({val})")

    def test_static_negate_methods(self):
        # static negate(Integer&)
        test_numbers = [0, 1, -1, 12345678901234567890, -98765432109876543210]
        for num in test_numbers:
            sign, words = to_words_str(num)
            out = run_cpp("negate_lvalue", sign, words)
            self.assertEqual(parse_cpp_output(out), -num)

        # static negate(Integer&&)
        for num in test_numbers:
            sign, words = to_words_str(num)
            out = run_cpp("negate_rvalue", sign, words)
            self.assertEqual(parse_cpp_output(out), -num)

    def test_plus_method(self):
        test_pairs = [
            (0, 0),
            (0, 12345),
            (12345, 0),
            (0xFFFFFFFFFFFFFFFF, 1),
            (0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF),
        ]
        # Fuzzing
        for _ in range(50):
            a = random.randint(0, 2**256)
            b = random.randint(0, 2**256)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            sign1, w1 = to_words_str(a)
            sign2, w2 = to_words_str(b)
            out = run_cpp("plus", sign1, w1, sign2, w2, "false")
            self.assertEqual(parse_cpp_output(out), a + b)

            # ignore carry
            out = run_cpp("plus", sign1, w1, sign2, w2, "true")
            len_w1 = len(w1.split(',')) if w1 else 1
            len_w2 = len(w2.split(',')) if w2 else 1
            max_len = max(len_w1, len_w2)
            limit = 1 << (64 * max_len)
            expected = (a + b) % limit
            self.assertEqual(parse_cpp_output(out), expected)

    def test_minus_method(self):
        test_pairs = [
            (0, 0),
            (100, 30),
            (30, 100),
            (0xFFFFFFFFFFFFFFFF, 1),
            (1, 0xFFFFFFFFFFFFFFFF),
        ]
        for _ in range(50):
            a = random.randint(0, 2**256)
            b = random.randint(0, 2**256)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            sign1, w1 = to_words_str(a)
            sign2, w2 = to_words_str(b)
            out = run_cpp("minus", sign1, w1, sign2, w2)
            self.assertEqual(parse_cpp_output(out), a - b)

    def test_operator_negate(self):
        test_vals = [0, 1, -1, 1234567890123456789, -9876543210987654321]
        for val in test_vals:
            sign, words = to_words_str(val)
            out = run_cpp("operator_negate", sign, words)
            self.assertEqual(parse_cpp_output(out), -val)

    def test_operator_tilde(self):
        test_vals = [
            0, 1, 0xFFFFFFFFFFFFFFFF,
            0x123456789ABCDEF0,
            0x9876543210FEDCBA123456789ABCDEF0
        ]
        for val in test_vals:
            sign, words = to_words_str(val)
            out = run_cpp("operator_tilde", sign, words)
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
                    sign, words = to_words_str(num)
                    
                    # << operator
                    out = run_cpp("operator_lshift", sign, words, shift)
                    self.assertEqual(parse_cpp_output(out), num << shift)
                    
                    # <<= operator
                    out = run_cpp("operator_lshift_assign", sign, words, shift)
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
                    sign, words = to_words_str(num)
                    
                    # C++ right shift for negative represents absolute value rshift with sign preserved
                    # 즉 C++의 >> 연산은 부호를 그대로 유지한 채 절댓값(array)에 대한 >> 연산을 수행한다.
                    # 따라서 파이썬의 abs(num) >> shift에 원래 부호를 입혀 예상 결과 계산
                    abs_shifted = abs(num) >> shift
                    expected = -abs_shifted if is_neg else abs_shifted
                    
                    # >> operator
                    out = run_cpp("operator_rshift", sign, words, shift)
                    self.assertEqual(parse_cpp_output(out), expected, f"Failed: {num} >> {shift}")
                    
                    # >>= operator
                    out = run_cpp("operator_rshift_assign", sign, words, shift)
                    self.assertEqual(parse_cpp_output(out), expected, f"Failed: {num} >>= {shift}")

    def test_operator_plus_and_plus_assign(self):
        test_pairs = [
            (0, 0), (12345, 67890), (-12345, 67890), (12345, -67890), (-12345, -67890),
            (0xFFFFFFFFFFFFFFFF, 1), (-0xFFFFFFFFFFFFFFFF, 1)
        ]
        for _ in range(50):
            a = random.randint(-2**256, 2**256)
            b = random.randint(-2**256, 2**256)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            sign1, w1 = to_words_str(a)
            sign2, w2 = to_words_str(b)
            
            # operator+
            out = run_cpp("operator_plus", sign1, w1, sign2, w2)
            self.assertEqual(parse_cpp_output(out), a + b, f"Failed: {a} + {b}")
            
            # operator+=
            out = run_cpp("operator_plus_assign", sign1, w1, sign2, w2)
            self.assertEqual(parse_cpp_output(out), a + b, f"Failed: {a} += {b}")

    def test_operator_minus_and_minus_assign(self):
        test_pairs = [
            (0, 0), (12345, 67890), (-12345, 67890), (12345, -67890), (-12345, -67890),
            (0xFFFFFFFFFFFFFFFF, 1), (-0xFFFFFFFFFFFFFFFF, 1)
        ]
        for _ in range(50):
            a = random.randint(-2**256, 2**256)
            b = random.randint(-2**256, 2**256)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            sign1, w1 = to_words_str(a)
            sign2, w2 = to_words_str(b)
            
            # operator-
            out = run_cpp("operator_minus", sign1, w1, sign2, w2)
            self.assertEqual(parse_cpp_output(out), a - b, f"Failed: {a} - {b}")
            
            # operator-=
            out = run_cpp("operator_minus_assign", sign1, w1, sign2, w2)
            self.assertEqual(parse_cpp_output(out), a - b, f"Failed: {a} -= {b}")

    def test_operator_le_and_ne_and_eq(self):
        test_pairs = [
            (0, 0), (1, 0), (0, 1), (-1, 0), (0, -1),
            (12345, 12345), (12345, 12346), (-12345, -12346),
            (258, 513), (513, 258)
        ]
        for _ in range(100):
            a = random.randint(-2**256, 2**256)
            b = random.randint(-2**256, 2**256)
            test_pairs.append((a, b))

        for a, b in test_pairs:
            sign1, w1 = to_words_str(a)
            sign2, w2 = to_words_str(b)
            
            # operator<=
            out = run_cpp("operator_le", sign1, w1, sign2, w2)
            expected = "true" if a <= b else "false"
            self.assertEqual(out, expected, f"Failed: {a} <= {b}")
            
            # operator!=
            out = run_cpp("operator_ne", sign1, w1, sign2, w2)
            expected = "true" if a != b else "false"
            self.assertEqual(out, expected, f"Failed: {a} != {b}")

            # operator==
            out = run_cpp("operator_eq", sign1, w1, sign2, w2)
            expected = "true" if a == b else "false"
            self.assertEqual(out, expected, f"Failed: {a} == {b}")

    def test_operator_assignment(self):
        # 1. operator=(const Integer&) copy assignment
        a, b = 12345678901234567890, -98765432109876543210
        sign1, w1 = to_words_str(a)
        sign2, w2 = to_words_str(b)
        out = run_cpp("operator_assign_copy", sign1, w1, sign2, w2)
        self.assertEqual(parse_cpp_output(out), b)

        # 2. operator=(Integer&&) move assignment
        out = run_cpp("operator_assign_move", sign1, w1, sign2, w2)
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
            sign, words = to_words_str(num)
            
            # to_string()
            out = run_cpp("to_string", sign, words)
            self.assertEqual(out, str(num), f"to_string failed for {num}")
            
            # operator<< (ostream output)
            out_ostream = run_cpp("ostream_out", sign, words)
            self.assertEqual(out_ostream, str(num), f"ostream output failed for {num}")

if __name__ == '__main__':
    unittest.main()
