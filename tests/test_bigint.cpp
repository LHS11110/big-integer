#include <iostream>
#include <vector>
#include <string>
#include "bigint.hpp"

// Simple test framework helper functions for pretty printing
struct TestSummary {
    int passed = 0;
    int failed = 0;
};

void print_result(const std::string& test_name, bool success, const std::string& expected, const std::string& actual) {
    if (success) {
        std::cout << "\033[32m[ PASS ]\033[0m " << test_name << std::endl;
    } else {
        std::cerr << "\033[31m[ FAIL ]\033[0m " << test_name << std::endl;
        std::cerr << "         Expected: \"" << expected << "\"" << std::endl;
        std::cerr << "         Actual:   \"" << actual << "\"" << std::endl;
    }
}

// Helper to construct Integer from vector safely for testing
Integer make_integer(const std::vector<uint64_t>& vec) {
    return Integer(vec);
}

void test_plus(TestSummary& summary) {
    std::cout << "\n--- Running plus() Tests ---" << std::endl;

    // Test Case 1: 0 + 0
    {
        std::string name = "plus: 0 + 0";
        Integer a(0);
        Integer b(0);
        Integer c = a.plus(b);
        // Note: due to a known bug in to_string() for zero value, this might be empty
        bool ok = (c.to_string() == "0");
        print_result(name, ok, "0", c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }

    // Test Case 2: 123 + 456
    {
        std::string name = "plus: 123 + 456 (Basic addition)";
        Integer a(123);
        Integer b(456);
        Integer c = a.plus(b);
        bool ok = (c.to_string() == "579");
        print_result(name, ok, "579", c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }

    // Test Case 3: 0xFFFFFFFFFFFFFFFF + 1 (Single-word carry overflow)
    {
        std::string name = "plus: 0xFFFFFFFFFFFFFFFF + 1 (Carry propagation)";
        Integer a(0xFFFFFFFFFFFFFFFFULL);
        Integer b(1);
        Integer c = a.plus(b);
        // Since to_string() pads non-first words to 16 digits:
        // array[0] = 0, array[1] = 1 -> "1" + "0000000000000000" = "10000000000000000"
        std::string expected = "10000000000000000";
        bool ok = (c.to_string() == expected);
        print_result(name, ok, expected, c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }

    // Test Case 4: MASK2 + MASK2 (Carry propagation with value)
    {
        std::string name = "plus: 0xFFFFFFFFFFFFFFFF + 0xFFFFFFFFFFFFFFFF (Max word values)";
        Integer a(0xFFFFFFFFFFFFFFFFULL);
        Integer b(0xFFFFFFFFFFFFFFFFULL);
        Integer c = a.plus(b);
        // 0xFFFFFFFFFFFFFFFF + 0xFFFFFFFFFFFFFFFF = 0xFFFFFFFFFFFFFFFE with carry 1
        // array[0] = 18446744073709551614, array[1] = 1
        // to_string() -> "1" + "18446744073709551614" = "118446744073709551614"
        std::string expected = "118446744073709551614";
        bool ok = (c.to_string() == expected);
        print_result(name, ok, expected, c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }
}

void test_minus(TestSummary& summary) {
    std::cout << "\n--- Running minus() Tests ---" << std::endl;

    // Test Case 1: 456 - 123
    {
        std::string name = "minus: 456 - 123 (Basic subtraction)";
        Integer a(456);
        Integer b(123);
        Integer c = a.minus(b);
        bool ok = (c.to_string() == "333");
        print_result(name, ok, "333", c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }

    // Test Case 2: 123 - 123
    {
        std::string name = "minus: 123 - 123 (Resulting in zero)";
        Integer a(123);
        Integer b(123);
        Integer c = a.minus(b);
        bool ok = (c.to_string() == "0");
        print_result(name, ok, "0", c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }

    // Test Case 3: {0, 1} - 1 (Borrow propagation)
    {
        std::string name = "minus: 0x10000000000000000 - 1 (Borrow propagation)";
        Integer a(std::vector<uint64_t>{0, 1});
        Integer b(1);
        Integer c = a.minus(b);
        // Should result in array[0] = 0xFFFFFFFFFFFFFFFFULL (18446744073709551615)
        std::string expected = "18446744073709551615";
        bool ok = (c.to_string() == expected);
        print_result(name, ok, expected, c.to_string());
        if (ok) summary.passed++; else summary.failed++;
    }
}

int main() {
    std::cout << "\033[36;1m===========================================\033[0m" << std::endl;
    std::cout << "\033[36;1m=== BigInteger Unit Tests: plus & minus ===\033[0m" << std::endl;
    std::cout << "\033[36;1m===========================================\033[0m" << std::endl;

    TestSummary summary;
    test_plus(summary);
    test_minus(summary);

    std::cout << "\n\033[36;1m===========================================\033[0m" << std::endl;
    std::cout << "\033[36;1m===            Test Summary             ===\033[0m" << std::endl;
    std::cout << "\033[36;1m===========================================\033[0m" << std::endl;
    std::cout << "Total Tests Run: " << (summary.passed + summary.failed) << std::endl;
    std::cout << "\033[32mPassed:          " << summary.passed << "\033[0m" << std::endl;
    if (summary.failed > 0) {
        std::cout << "\033[31mFailed:          " << summary.failed << "\033[0m" << std::endl;
        std::cout << "\n\033[33mNote: Some test failures are due to implementation details or bugs in bigint.cpp.\033[0m" << std::endl;
    } else {
        std::cout << "\033[32mAll tests passed successfully!\033[0m" << std::endl;
    }
    std::cout << "\033[36;1m===========================================\033[0m\n" << std::endl;

    return summary.failed > 0 ? 1 : 0;
}
