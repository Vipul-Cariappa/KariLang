#include <iostream>
#include <memory>

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::unique_ptr<T> &x) {
    return os << *x;
}
