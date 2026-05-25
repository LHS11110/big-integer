#include "bigint.hpp"
#include <iostream>

auto main(void) -> int {
    Integer a({23487293479, 234798118924912378});
    a = a << 67;

    std::cout << (234798118924912378 << 3) << '\n';

    return 0;
}
