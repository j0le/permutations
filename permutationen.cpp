#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

static_assert(std::is_same_v<std::size_t, decltype(sizeof(0))>);
typedef std::conditional_t<std::is_signed_v<char>, signed char, unsigned char>
    underlying_char_type;

static std::optional<size_t> letter_to_index(char c, std::size_t size) {
    if (c < 'A' || c > 'Z')
        return std::nullopt;

    std::size_t index = c - 'A';
    if (std::cmp_greater_equal(index, size))
        return std::nullopt;
    return std::make_optional(index);
}

static std::optional<char> index_to_char(size_t index, std::size_t size) {
    if (std::cmp_greater_equal(index, size))
        return std::nullopt;

    auto sum = 'A' + index;

    static_assert(std::is_same_v<decltype(index), decltype(sum)>);
    if (std::cmp_less(sum, index))
        return std::nullopt; // overflow

    underlying_char_type number_z = 'Z';
    if (std::cmp_greater(sum, number_z))
        return std::nullopt;
    char c = sum;
    return c;
}

static std::optional<std::string>
apply_permutations_onto_another(std::string_view a, std::string_view b) {
    if (a.size() != b.size())
        return std::nullopt;
    std::size_t size = a.size();

    std::string result(size, '\0');
    for (std::size_t i = 0; i < size; ++i) {
        auto new_index_opt = letter_to_index(a[i], size);
        if (!new_index_opt)
            return std::nullopt;
        auto new_index = *new_index_opt;
        result[i] = b[new_index];
    }
    return result;
}

static std::optional<std::string>
get_other_permutation_representation(std::string_view view) {
    // Print permutations like in the book "Elementar(st)e Gruppentheorie"
    // by Tobias Glosauer,
    // Chapter 3 "Gruppen ohne Ende",
    // Section 3.2 "Symetrische Gruppen", page 51
    std::string ret{};
    const constexpr auto diff = 'Z' - 'A' + '\01';
    static_assert(std::cmp_equal(26, diff));
    assert(std::cmp_less_equal(view.size(), diff));
    auto found = std::make_unique<bool[]>(view.size());
    for (std::size_t i = 0z; i < view.size(); ++i) {
        if (found[i]) {
            continue;
        }
        found[i] = true;
        char current_letter = 'A' + i;
        ret += '(';
        ret += current_letter;
        size_t next_index = i;
        while (true) {
            char next_letter = view[next_index];
            if (next_letter < 'A' || next_letter > 'Z')
                return std::nullopt;
            if (next_letter == current_letter)
                break;
            next_index = next_letter - 'A';
            if (next_index >= view.size())
                return std::nullopt;
            found[next_index] = true;
            ret += next_letter;
        }
        ret += ')';
    }
    return ret;
}
static void print_permutation_differently(std::FILE *stream,
                                          std::string_view view) {
    auto opt = get_other_permutation_representation(view);
    if (!opt)
        throw std::exception();
    std::print(stream, "{} - {}", view, *opt);
}
static void print_permutation_differently(std::FILE *stream,
                                          std::span<char> span) {
    std::string_view view(span.data(), span.size());
    print_permutation_differently(stream, view);
}

static void print_span(std::span<char> span) {
    std::string_view view(span.data(), span.size());
    std::print("|{}|\n", view);
}

static std::string get_identity_permutation(std::size_t places) {
    std::string str(places, '\0');
    str.assign_range(std::ranges::iota_view{'A'} | std::views::take(places));
    return str;
}

static std::optional<std::size_t> get_order(std::string_view view) {
    const std::string identity_permutation =
        get_identity_permutation(view.size());
    std::string str = identity_permutation;

    std::size_t ret = 0;
    do {
        auto string_opt = apply_permutations_onto_another(view, str);
        if (!string_opt) {
            return std::nullopt;
        }
        str = std::move(*string_opt);
        ret += 1;
    } while (str != identity_permutation);
    return ret;
}

static void print_all_powers(std::FILE *stream, std::string_view view) {
    const std::string identity_permutation =
        get_identity_permutation(view.size());
    std::string str = identity_permutation;

    do {
        auto string_opt = apply_permutations_onto_another(view, str);
        if (!string_opt) {
            std::println(stderr, "error");
            break;
        }
        str = std::move(*string_opt);
        print_permutation_differently(stream, std::string_view{str});
        std::print(stream, ",  ");
    } while (str != identity_permutation);
}

static void print_all_powers(std::FILE *stream, std::span<char> span) {
    std::string_view view(span.data(), span.size());
    return print_all_powers(stream, view);
}

template <typename T>
concept bool_or_void = std::same_as<T, bool> || std::same_as<T, void>;

template <bool_or_void ReturnTypeOfCallBack>
[[nodiscard]] static ReturnTypeOfCallBack calc_permutation(
    std::function<ReturnTypeOfCallBack(std::string_view)> call_back,
    std::span<char> all, std::span<char> filled, std::span<char> unfilled) {
    std::size_t sum = filled.size() + unfilled.size();
    assert(std::cmp_equal(sum, all.size()));
    assert(filled.empty() || (all.begin() == filled.begin()));
    assert(unfilled.empty() ||
           (all.begin() + filled.size()) == unfilled.begin());
    if (std::cmp_equal(unfilled.size(), 0)) {
        std::string_view view(filled.data(), filled.size());
        return call_back(view);
    }
    char c = 'A';
    bool found = false;
    const char last_char = static_cast<char>('A' + (sum - 1));
    while (c <= last_char) {
        while (c <= last_char) {
            found = false;
            for (const char used : filled) {
                if (c == used) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
            c++;
        }
        if (c > last_char) {
            break;
        }
        assert(!found);
        unfilled[0] = c;
        auto call_recursive = [&] {
            return calc_permutation(call_back, all,
                                    all.first(filled.size() + 1),
                                    all.last(unfilled.size() - 1));
        };
        if constexpr (std::is_same_v<ReturnTypeOfCallBack, void>) {
            call_recursive();
        } else if (!call_recursive()) {
            return false;
        }

        c++;
    }
    return static_cast<ReturnTypeOfCallBack>(true);
}

[[nodiscard]] bool print_permutation(std::uint32_t places) {
    const auto max_number_of_digits = 'Z' - 'A' + 1z;
    if (std::cmp_greater(places, max_number_of_digits)) {
        return false;
    }
    std::string str(places, '\0');
    std::span all(str.data(), str.length());

    auto print = [](std::string_view view) -> void {
        //print_span(filled);
        //print_permutation_differently(stdout, filled);
        print_all_powers(stdout, view);
        std::println(stdout, "");
    };

    calc_permutation<void>(print, all, all.first(0), all);
    return true;
}

template <std::integral Integer> Integer fakultät(const Integer numb) {
    Integer result = 1;
    for (Integer i = 1; i <= numb; ++i) {
        result *= i;
    }
    return result;
}

template <std::ranges::range R>
    requires std::same_as<std::ranges::range_value_t<R>, std::string_view>
[[nodiscard]] static bool print_table(R perms, std::uint32_t places) {
    auto print_cell = [](std::string_view perm, bool header = false) -> bool {
        auto hover_text = perm;
        auto display_text_opt = get_other_permutation_representation(perm);
        if (!display_text_opt) {
            std::println(stderr, "this is the fucked up thing: {}", perm);
            return false;
        }
        auto order_opt = get_order(perm);
        if (!order_opt) {
            return false;
        }
        std::print("<t{5} class=\"{1}{2}\" data-perm=\"{1}\" title=\"{3}, order: {4}\">{0}</t{5}>",
                   *display_text_opt, perm,
                   (header ? " table_header" : ""), hover_text, *order_opt,
                   (header ? 'h' : 'd'));
        return true;
    };

    auto print_row = [&](std::string_view perm,
                         bool header_row = false) -> bool {
        for (std::string_view perm_column : perms) {
            auto opt = apply_permutations_onto_another(perm, perm_column);
            if (!opt || !print_cell(*opt, header_row)) {
                return false;
            }
        }
        std::println("</tr>");
        return true;
    };

    std::println("<table>");
    // print header of table
    std::print("<thead>\n<tr><th></th>");
    const auto identity = get_identity_permutation(places);
    if (!print_row(identity, true))
        return false;
    std::println("</thead>");

    // print bulk of the table
    std::println("<tbody>");
    for (std::string_view perm_row : perms) {
        std::print("<tr>");
        print_cell(perm_row, true);
        if (!print_row(perm_row))
            return false;
    }
    std::println("</tbody></table>");
    return true;
}

template <std::ranges::range R, std::integral UInt32>
    requires std::same_as<UInt32, std::uint32_t>
[[nodiscard]] static bool print_css(R perms, UInt32 places,
                                    std::size_t number_of_permutations) {
    std::println("<style>");
    size_t i = 0;
    bool first = true;
    for (std::string_view perm : perms) {
        auto order_opt = get_order(perm);
        if (!order_opt) {
            return false;
        }
        std::size_t color_by_order = 360z * (order_opt.value() - 1z) / places;
        std::size_t color = false ? color_by_order : i;
        std::println("tr:not(:hover) .{}:not(.selected_elm){{\n"
                     "{}background-color: hsl( {}deg 75% 75% ){};\n"
                     "}}",
                     perm, (first ? "/*" : ""), color, (first ? "*/" : ""));
        first = false;
        i += (360 / number_of_permutations);
    }
    std::println("</style>");
    std::println(R"(<link rel="stylesheet" href="./style.css" />)");
    return true;
}

template <std::ranges::range R, std::integral UInt32>
    requires std::same_as<UInt32, std::uint32_t>
[[nodiscard]] static bool print_table_permuted(R perms, UInt32 places) {
    assert(std::cmp_greater_equal(perms.size(), 1));
    std::string str(perms.size(), '\0');
    std::span all(str.data(), str.length());

    size_t counter = 0;
    auto permute_table_and_print = [&](std::string_view view) -> bool {
        std::vector<std::string_view> new_order{};
        new_order.reserve(perms.size());
        assert(view.size() == perms.size());
        for (char c : view) {
            auto index_opt = letter_to_index(c, view.size());
            if (!index_opt) {
                return false;
            }
            auto index = *index_opt;
            new_order.push_back(std::string_view{perms[index]});
        }
        std::println("<br/><p>Tabelle {}, {}</p>", counter, view);
        if (!print_table(new_order, places))
            return false;
        counter++;
        return true;
    };

    return calc_permutation<bool>(permute_table_and_print, all, all.first(0),
                                  all);
}

static bool compare_by_order(std::string_view a, std::string_view b) {
    auto order_a_opt = get_order(a);
    auto order_b_opt = get_order(b);
    if (order_a_opt.has_value() && order_b_opt.has_value())
        return *order_a_opt < *order_b_opt;
    else if (order_a_opt.has_value() == order_b_opt.has_value())
        return false;
    else if (!order_a_opt.has_value())
        return true;
    else
        return false;
}

[[nodiscard]] bool print_group_table(std::uint32_t places,
                                     bool permute_table = false) {
    const auto max_number_of_digits = 'Z' - 'A' + 1z;
    if (std::cmp_greater(places, max_number_of_digits)) {
        return false;
    }
    std::string str(places, '\0');
    std::span all(str.data(), str.length());

    std::size_t number_of_permutations = fakultät(static_cast<size_t>(places));

    std::vector<std::string> perms{};
    perms.reserve(number_of_permutations);

    auto put_into_vector = [&](std::string_view view) -> void {
        perms.push_back(std::string{view});
    };

    calc_permutation<void>(put_into_vector, all, all.first(0), all);

    assert(number_of_permutations == perms.size());

    std::println("<!DOCTYPE html>\n<html>\n<head>");

    std::println(R"(<script src="./script.js" defer></script>)");

    if (!print_css(perms, places, number_of_permutations))
        return false;
    std::println("</head>\n<body>");
    std::println("<p>number of permutations: {}</p>", number_of_permutations);

    if (permute_table) {
        if (!print_table_permuted(perms, places))
            return false;
    } else {
        auto range_of_string_views =
            perms | std::views::transform(
                        []<typename T>(T &&string) -> std::string_view {
                            return std::string_view{std::forward<T>(string)};
                        });
        auto vector_of_string_views =
            range_of_string_views | std::ranges::to<std::vector>();
        std::ranges::sort(vector_of_string_views, compare_by_order);
        if (!print_table(vector_of_string_views, places))
            return false;
        std::println("<br/><p>unsorted:</p>");
        if (!print_table(range_of_string_views, places))
            return false;
    }

    std::println("</body>\n</html>");
    return true;
}

static void print_binary_permutation(std::span<char> all, std::span<char> rest,
                                     std::size_t part) {
    if (rest.empty()) {
        //if(!all.empty()){
        print_span(all);
        //}
        return;
    }
    assert(std::cmp_greater_equal(rest.size(), part));
    if (std::cmp_greater(part, 0z)) {
        rest[0] = 'x';
        print_binary_permutation(all, rest.subspan(1z), part - 1z);
    }
    if (std::cmp_greater(rest.size(), part)) {
        rest[0] = '_';
        print_binary_permutation(all, rest.subspan(1z), part);
    }
}

[[nodiscard]] bool print_binary_permutation(std::uint32_t places,
                                            std::uint32_t part) {
    if (std::cmp_less(places, part)) {
        return false;
    }
    std::string str(places, '\0');
    std::span<char> span(str.data(), str.length());
    print_binary_permutation(span, span, part);
    return true;
}

static void print_ternary_permutation(std::span<char> all, std::span<char> rest,
                                      std::size_t a, std::size_t b,
                                      std::size_t c) {
    if (rest.empty()) {
        print_span(all);
        return;
    }
    assert(std::cmp_equal(rest.size(), a + b + c));
    if (a > 0z) {
        rest[0] = 'a';
        print_ternary_permutation(all, rest.subspan(1z), a - 1z, b, c);
    }
    if (b > 0z) {
        rest[0] = 'B';
        print_ternary_permutation(all, rest.subspan(1z), a, b - 1z, c);
    }
    if (c > 0z) {
        rest[0] = ' ';
        print_ternary_permutation(all, rest.subspan(1z), a, b, c - 1z);
    }
}

[[nodiscard]] bool print_ternary_permutation(std::uint32_t a, std::uint32_t b,
                                             std::uint32_t c) {
    static_assert(sizeof(std::size_t) >= sizeof(std::uint32_t));
    auto ab = a + b;
    if (std::cmp_less(ab, a)) {
        return false; // overflow
    }
    auto abc = ab + c;
    if (std::cmp_less(abc, ab)) {
        return false; // overflow
    }
    size_t sum = abc;
    std::string str(abc, '\0');
    std::span<char> span(str.data(), str.length());
    print_ternary_permutation(span, span, a, b, c);
    return true;
}

void check_expect(std::string_view a, std::string_view b,
                  std::string_view expected) {
    auto result = apply_permutations_onto_another(a, b);
    if (!result)
        throw std::exception();

    auto res = result.value();
    if (result == expected)
        std::println(stderr, "{} x {} = {} (correct)", a, b, res);
    else {
        std::println(stderr, "{} x {} = {}, expected {}", a, b, res, expected);
        assert(res == expected);
        throw std::exception();
    }
}

int main() {

    check_expect("ABC", "ABC", "ABC");
    check_expect("ABC", "CAB", "CAB");
    check_expect("CAB", "ABC", "CAB");

    std::string murks = "BCA";
    print_all_powers(stderr, std::string_view{murks});

    if (!print_group_table(4)) {
        std::print(stderr, "error");
        return 1;
    }
    //print_binary_permutation(10,5); // n over k, binomal coefficient
    //print_ternary_permutation(1,1,5);

    return 0;
}
