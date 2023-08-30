#include <iostream>
#include <vector>
#include <fmt/core.h>
#include <concepts>
#include <type_traits>
#include <random>
#include <algorithm>
#include <numbers>

template<typename T>
concept number = std::integral<T> or std::floating_point<T>;

void print(number auto val) {
    std::cout << fmt::format("number = {}\n", val);
}

void print(auto val) {
    std::cout << fmt::format("not number = {}\n", val);
}

void print(std::integral auto val) {
    std::cout << fmt::format("integral = {}\n", val);
}



template<typename T> constexpr bool is_a = true;
template<typename T> constexpr bool is_b = true;

template<typename T> concept A = is_a<T>;
template<typename T> concept badAB = is_a<T> and is_b<T>;
template<typename T> concept goodAB = A<T> and is_b<T>;

template<A T> void func1(T) {
    std::cout << "A\n";
}

template<badAB T> void func1(T) {
    std::cout << "badAB\n";
}

template<A T> void func2(T) {
    std::cout << "A\n";
}

template<goodAB T> void func2(T) {
    std::cout << "goodAB\n";
}


int main() {
    std::cout << fmt::format("{:=^80}\n", "<examples>");

    print(42);
    print(std::numbers::phi);
    print("string");

    // func1(0);
    func2(0);

    std::cout << fmt::format("{:=^80}\n", "</examples>");
}