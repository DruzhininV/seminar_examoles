//================================
//https://godbolt.org/z/6v77M8Gza
//================================


#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <boost/circular_buffer.hpp>
#include <fmt/core.h>
#include <concepts>
#include <type_traits>
#include <random>
#include <algorithm>
#include <ranges>


template<typename T>
concept std_like_container = std::copyable<T>
                             and requires (T container) {
    typename T::value_type;
//    typename T::allocator_type;
    typename T::size_type;
    typename T::difference_type;
    typename T::reference;
    typename T::const_reference;
    typename T::pointer;
    typename T::const_pointer;
    typename T::iterator;
    typename T::const_pointer;
    typename T::reverse_iterator;
    typename T::const_reverse_iterator;

    { container.begin() } -> std::same_as<typename T::iterator>;
    { container.end() } -> std::same_as<typename T::iterator>;
    { container.cbegin() } -> std::same_as<typename T::const_iterator>;
    { container.cend() } -> std::same_as<typename T::const_iterator>;
    { container.size() } noexcept -> std::same_as<typename T::size_type>;
    { container.empty() } noexcept -> std::same_as<bool>;
    { container.front() } -> std::same_as<typename T::reference>;
    { container.back() } -> std::same_as<typename T::reference>;
    requires requires (T const& constContainer) {
        { constContainer.begin() } -> std::same_as<typename T::const_iterator>;
        { constContainer.end() } -> std::same_as<typename T::const_iterator>;
        { constContainer.front() } -> std::same_as<typename T::const_reference>;
        { constContainer.back() } -> std::same_as<typename T::const_reference>;
    };
};

static_assert(std_like_container<std::vector<int>>);
static_assert(std_like_container<std::list<int>>);
static_assert(std_like_container<std::array<int,10>>);
static_assert(std_like_container<boost::circular_buffer<int>>);



template<std_like_container Container>
auto make_test_data(std::unsigned_integral auto size) {
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution dest(1, 100);
    Container container(size);
    std::generate(container.begin(), container.end(), [&]{ return dest(gen);});
    return container;
}

void print(std_like_container auto const& container) {
std::cout << "{ ";
std::ranges::for_each(container, [&](auto val) {
std::cout << val << ' ';
});
std::cout << " }\n";
}


int main() {
    std::cout << fmt::format("{:=^80}\n", "<examples>");

    std_like_container auto container{ make_test_data<std::vector<int>>(25u)};
    print(container);

    std::cout << fmt::format("{:=^80}\n", "</examples>");
}