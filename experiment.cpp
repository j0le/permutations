#include <print>
#include <span>
#include <type_traits>

struct experiment_view;
struct experiment_t {
    int i{};
    int get_value() { return this->i; }

    experiment_t(int i) : i{i} { std::println("normal constructor"); }

    experiment_t(const experiment_t &other) : i{other.i} {
        std::println("copy ctor");
    }

    experiment_t(experiment_t &&other) : i{other.i} {
        std::println("move ctor");
    }

    experiment_t &operator=(const experiment_t &other) {
        std::println("copy assign");
        //return *this = experiment_t(other);
        this->i = other.i;
        return *this;
    }
    experiment_t &operator=(experiment_t &&other) {
        this->i = other.i;
        std::println("copy assign");
        return *this;
    }

    ~experiment_t() = default;

    constexpr operator experiment_view();
};

struct experiment_view {
    const experiment_t *ptr{};

    //experiment_view(const experiment_view&) = default;
    //experiment_view(experiment_view&&) = default;
    //
    //experiment_view(const experiment_t &e) : ptr{&e} {
    //    std::println("view ctr");
    //}

    int get_value() { return ptr->i; }
};
static_assert(std::is_aggregate_v<experiment_view>);

constexpr experiment_t::operator experiment_view() {
    std::println("conversion");
    return experiment_view{this};
}

template <std::ranges::range R> void experiment_sub_fn(R range) {
    for (experiment_view x : range) {
        std::println("{}", x.get_value());
    }
}

void experiment_std_array() {
    std::array<experiment_t, 3> arr = {1, 2, 3};

    std::println("--------");
    experiment_sub_fn(std::span{arr});
}

void experiment_std_vector() {
    std::vector<experiment_t> vec = {1, 2};
    std::println("{}", vec.capacity());
    std::println("----\nreserve");
    vec.reserve(10);
    std::println("----");
    vec.push_back(experiment_t{3});
    std::println("--------");
    experiment_sub_fn(std::span{vec});
}

int main() {
    experiment_std_array();
    std::println("--------------------");
    experiment_std_vector();
    return 0;
}
