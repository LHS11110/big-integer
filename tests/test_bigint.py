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

    def setUp(self):
        self._start_time = time.time()

    def tearDown(self):
        duration = time.time() - self._start_time
        print(f"\n[TIME] {self.id().split('.')[-1]}: {duration:.4f}s", flush=True)

    def test_construct_default(self):
        for _ in range(1000):
            out = run_cpp("construct_default")
            self.assertEqual(parse_cpp_output(out), 0)

    def test_construct_uint64(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            val = abs(a) & 0xFFFFFFFFFFFFFFFF
            is_neg = a < 0
            out = run_cpp("construct_uint64", val, "true" if is_neg else "false")
            expected = -val if is_neg else val
            self.assertEqual(parse_cpp_output(out), expected)

    def test_construct_vector(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            abs_val = abs(a)
            _, words_str = to_words_str(abs_val)
            out = run_cpp("construct_vector", words_str)
            self.assertEqual(parse_cpp_output(out), abs_val)

    def test_construct_copy(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            new_neg = random.choice([True, False])
            out = run_cpp("construct_copy", str(a), "true" if new_neg else "false")
            expected = -abs(a) if new_neg else abs(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_construct_move(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("construct_move", str(a))
            self.assertEqual(parse_cpp_output(out), a)

    def test_construct_move_sign(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            new_neg = random.choice([True, False])
            out = run_cpp("construct_move_sign", str(a), "true" if new_neg else "false")
            expected = -abs(a) if new_neg else abs(a)
            self.assertEqual(parse_cpp_output(out), expected)

    def test_construct_string(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("construct_string", str(a))
            self.assertEqual(parse_cpp_output(out), a)

    def test_negate_lvalue(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("negate_lvalue", str(a))
            self.assertEqual(parse_cpp_output(out), -a)

    def test_negate_rvalue(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("negate_rvalue", str(a))
            self.assertEqual(parse_cpp_output(out), -a)

    def test_abs_lvalue(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("abs_lvalue", str(a))
            self.assertEqual(parse_cpp_output(out), abs(a))

    def test_abs_rvalue(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("abs_rvalue", str(a))
            self.assertEqual(parse_cpp_output(out), abs(a))

    def test_plus(self):
        for _ in range(1000):
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
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_digits(len_a)
            b = generate_random_digits(len_b)
            out = run_cpp("minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b)

    def test_operator_negate(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("operator_negate", str(a))
            self.assertEqual(parse_cpp_output(out), -a)

    def test_operator_tilde(self):
        for _ in range(1000):
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

    def test_operator_lshift(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_lshift", str(a), shift)
            self.assertEqual(parse_cpp_output(out), a << shift)

    def test_operator_rshift(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_rshift", str(a), shift)
            abs_shifted = abs(a) >> shift
            expected = -abs_shifted if a < 0 else abs_shifted
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_lshift_assign(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_lshift_assign", str(a), shift)
            self.assertEqual(parse_cpp_output(out), a << shift)

    def test_operator_rshift_assign(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            shift = random.randint(200, 300)
            out = run_cpp("operator_rshift_assign", str(a), shift)
            abs_shifted = abs(a) >> shift
            expected = -abs_shifted if a < 0 else abs_shifted
            self.assertEqual(parse_cpp_output(out), expected)

    def test_operator_plus(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_plus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b)

    def test_operator_plus_assign(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_plus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a + b)

    def test_operator_minus(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_minus", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b)

    def test_operator_minus_assign(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_minus_assign", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a - b)

    def test_operator_le(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_le", str(a), str(b))
            expected = "true" if a <= b else "false"
            self.assertEqual(out, expected)

    def test_operator_lt(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_lt", str(a), str(b))
            expected = "true" if a < b else "false"
            self.assertEqual(out, expected)

    def test_operator_ne(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_ne", str(a), str(b))
            expected = "true" if a != b else "false"
            self.assertEqual(out, expected)

    def test_operator_eq(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_eq", str(a), str(b))
            expected = "true" if a == b else "false"
            self.assertEqual(out, expected)

    def test_operator_assign_copy(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_assign_copy", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), b)

    def test_operator_assign_move(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("operator_assign_move", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), b)

    def test_to_string(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("to_string", str(a))
            self.assertEqual(out, str(a))

    def test_ostream_out(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("ostream_out", str(a))
            self.assertEqual(out, str(a))

    def test_size(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            a = generate_random_signed(len_a)
            out = run_cpp("size", str(a))
            _, words = to_words_str(a)
            expected_size = len(words.split(",")) if words else 1
            self.assertEqual(parse_cpp_output(out), expected_size)

    def test_karatsuba_mul(self):
        for _ in range(1000):
            len_a = random.randint(100, 200)
            len_b = random.randint(100, 200)
            a = generate_random_signed(len_a)
            b = generate_random_signed(len_b)
            out = run_cpp("karatsuba_mul", str(a), str(b))
            self.assertEqual(parse_cpp_output(out), a * b)

if __name__ == '__main__':
    unittest.main()
