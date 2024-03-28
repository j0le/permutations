#include <algorithm>
#include <bitset>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <optional>
#include <print>
#include <ranges>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace permutations {

namespace concepts {

// suffix "_c" is for "concept"

template <typename T>
concept bool_or_void_c = std::same_as<T, bool> || std::same_as<T, void>;

template <typename R>
concept range_of_string_views_c =
    ::std::ranges::range<R> &&
    std::same_as<std::ranges::range_value_t<R>, std::string_view>;

template <typename R>
concept range_of_string_view_likes_c =
    ::std::ranges::range<R> &&
    std::convertible_to<std::ranges::range_value_t<R>, std::string_view>;

template <typename T>
concept uint32_c = std::same_as<T, std::uint32_t>;

template <typename T>
concept char_c = std::same_as<T, char>;

// https://stackoverflow.com/a/67603594/6275838
template <typename From, typename To>
concept non_narrowing = requires(From f) { To{f}; };

} // namespace concepts

static_assert(std::is_same_v<std::size_t, decltype(sizeof(0))>);
typedef std::conditional_t<std::is_signed_v<char>, signed char, unsigned char>
    underlying_char_type;

static std::optional<size_t> letter_to_index(concepts::char_c auto c,
                                             std::size_t size) {
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

class PermutationException : public std::exception {};

struct PermutationView;
class Permutation {
  public:
    typedef uint32_t uint_t;
    typedef std::span<const uint_t> readonly_span;
    typedef std::span<uint_t> span;

  private:
    std::unique_ptr<uint_t[]> m_data{};
    span m_span{};

  public:
    Permutation(uint_t places, bool make_identity_perm = false)
        : m_data{new uint_t[places]{}}, m_span{m_data.get(), places} {

        if (make_identity_perm) {
            auto range =
                std::ranges::iota_view{uint_t{}} | std::views::take(places);
            std::ranges::copy(range, m_span.begin());
        }
    }
    Permutation(std::initializer_list<uint_t> init)
        : m_data{new uint_t[init.size()]{}}, m_span{m_data.get(), init.size()} {

        std::ranges::copy(init, m_span.begin());
    }
    Permutation(std::ranges::sized_range auto &&range)
        requires std::same_as<std::ranges::range_value_t<decltype(range)>,
                              uint_t>
        : m_data{new uint_t[range.size()]{}},
          m_span{m_data.get(), range.size()} {
        std::ranges::copy(range, m_span.begin());
    }

    Permutation(const Permutation &other)
        : m_data{new uint_t[other.m_span.size()]{}},
          m_span{m_data.get(), other.m_span.size()} {
        std::ranges::copy(other.m_span, m_span.begin());
    }
    Permutation(Permutation &&) = default;

    Permutation &operator=(const Permutation &other) {
        *this = Permutation(other);
        return *this;
    }
    Permutation &operator=(Permutation &&) = default;

    ~Permutation() = default;

    constexpr span get_span() { return m_span; }
    constexpr readonly_span get_readonly_span() const {
        return std::span<const uint_t>{this->m_span};
    }

    constexpr std::string to_string() const;

    constexpr operator readonly_span() const { return get_readonly_span(); }
    constexpr operator span() { return get_span(); }
    constexpr operator PermutationView() const;

    constexpr std::size_t size() const { return m_span.size(); }
};

struct PermutationView : public Permutation::readonly_span {
  private:
    typedef Permutation::readonly_span base;

  public:
    // ctor
    constexpr PermutationView() : base() {}
    //constexpr PermutationView(const Permutation& perm)
    //    : base(perm.get_readonly_span()) {}

    // copy ctor, assignment
    constexpr PermutationView(const PermutationView &) = default;
    constexpr PermutationView &operator=(const PermutationView &) = default;

    // move ctor, assignment
    //PermutationView(PermutationView&&) = default;
    //PermutationView& operator=(PermutationView&&) = default;

    // ctor, assignment from base
    constexpr PermutationView(const base &b) : base(b) {}
    constexpr PermutationView &operator=(const base &b) {
        this->base::operator=(b);
        return *this;
    }

    // dtor
    constexpr ~PermutationView() = default;

    constexpr base get_readonly_span() { return *this; }

    constexpr bool operator==(const PermutationView &other) const {
        if (other.size() != this->size())
            return false;
        auto other_it = other.begin();
        for (auto i : *this) {
            if (i != *(other_it++))
                return false;
        }
        return true;
    }

    constexpr std::string to_string() const {
        const std::size_t size = this->size();
        auto view = *this | std::views::transform(
                                [size](Permutation::uint_t i) -> char {
                                    auto opt = index_to_char(i, size);
                                    if (!opt)
                                        throw PermutationException();
                                    return *opt;
                                });
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(view)>, char>);
        return view | std::ranges::to<std::string>();
    }
};

constexpr Permutation::operator PermutationView() const {
    return PermutationView{this->get_readonly_span()};
}

constexpr std::string Permutation::to_string() const {
    return this->operator PermutationView().to_string();
}

std::optional<Permutation> str_to_perm(std::string_view view) {
    std::optional<Permutation> perm(std::in_place, view.size());
    auto span = perm->get_span();
    auto it = span.begin();
    auto end = span.end();
    for (char c : view) {
        if (it == end)
            return std::nullopt;
        auto i_opt = letter_to_index(c, view.size());
        if (!i_opt)
            return std::nullopt;
        *it = *i_opt;
        ++it;
    }
    assert(it == end);
    return perm;
}

namespace concepts {
template <typename UINT>
concept Permutation_uint_c = std::same_as<UINT, Permutation::uint_t>;

template <typename R>
concept range_of_PermutationViews_c =
    ::std::ranges::range<R> &&
    std::same_as<std::ranges::range_value_t<R>, PermutationView>;

template <typename R>
concept range_of_PermutationView_likes_c =
    ::std::ranges::range<R> &&
    std::convertible_to<std::ranges::range_value_t<R>, PermutationView>;
} //namespace concepts

} // namespace permutations

/*
// https://fmt.dev/latest/api.html#formatting-user-defined-types
// https://en.cppreference.com/w/cpp/utility/format/formatter
// template specialization must be in global namespace
template <>
struct std::formatter<permutations::PermutationView, char> {

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext &ctx) {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;

        if (*it != '}')
            throw std::format_error("Invalid format args for PermutationView.");

        return it;
    }

    template <typename FmtContext>
    FmtContext::iterator format(const permutations::PermutationView &perm_view,
                                FmtContext &ctx) {
        using namespace permutations;
        const std::size_t size = perm_view.size();
        auto view =
            perm_view |
            std::views::transform([size](Permutation::uint_t i) -> char {
                auto opt = index_to_char(i, size);
                if (!opt)
                    throw PermutationException();
                return *opt;
            });
        static_assert(
            std::same_as<std::ranges::range_value_t<decltype(view)>, char>);
        std::ranges::copy(view, ctx.out());
    }
};*/

namespace permutations {

// Composition of permutations as if they are functions:
// a ∘ b
// (a∘b)(i) = a(b(i))
static std::optional<Permutation>
compose_permutations(Permutation::readonly_span a,
                     Permutation::readonly_span b) {
    if (a.size() != b.size())
        return std::nullopt;
    std::size_t size = a.size();

    std::optional<Permutation> result(std::in_place, size);
    auto span = result->get_span();
    for (std::size_t i = 0; i < size; ++i) {
        auto new_index = b[i]; // As if `b` was a (mathematical) function: b(i).
        if (std::cmp_greater_equal(new_index, size))
            return std::nullopt;
        span[i] = a[new_index];
    }
    return result;
}

Permutation inverse(const Permutation::readonly_span a) {
    Permutation result(a.size());
    auto span = result.get_span();

    for (Permutation::uint_t i{}; std::cmp_less(i, a.size()); ++i) {
        auto j = a[i];
        assert(std::cmp_less(j, a.size()));
        span[j] = i;
    }
    return result;
}

static std::optional<std::string>
get_other_permutation_representation(const Permutation::readonly_span span) {
    // Print permutations like in the book "Elementar(st)e Gruppentheorie"
    // by Tobias Glosauer,
    // Chapter 3 "Gruppen ohne Ende",
    // Section 3.2 "Symetrische Gruppen", page 51
    std::optional<std::string> ret(std::in_place);
    const constexpr auto max_size = 'Z' - 'A' + '\01';
    static_assert(std::cmp_equal(26, max_size));
    assert(std::cmp_less_equal(span.size(), max_size));
    //auto found = std::make_unique<bool[]>(view.size());
    std::bitset<max_size> found{};
    for (std::size_t i = 0z; i < span.size(); ++i) {
        if (found.test(i)) {
            continue;
        }
        found.set(i);
        char current_letter = 'A' + i;
        *ret += '(';
        *ret += current_letter;
        size_t next_index = i;
        while (true) {
            next_index = span[next_index];
            std::optional<char> next_letter_opt =
                index_to_char(next_index, span.size());
            if (!next_letter_opt)
                return std::nullopt;
            if (next_index == i)
                break;
            found.set(next_index);
            *ret += *next_letter_opt;
        }
        *ret += ')';
    }
    return ret;
}
static void print_permutation_differently(std::FILE *stream,
                                          Permutation::readonly_span view) {
    auto opt = get_other_permutation_representation(view);
    if (!opt)
        throw std::exception();
    std::print(stream, "{} - {}", PermutationView{view}.to_string(), *opt);
}

static void print_span(std::span<char> span) {
    std::string_view view(span.data(), span.size());
    std::print("|{}|\n", view);
}

static Permutation get_identity_permutation(std::size_t places) {
    return Permutation(places, true);
}

static std::optional<std::size_t> get_order(Permutation::readonly_span view) {
    const Permutation identity_permutation =
        get_identity_permutation(view.size());
    const PermutationView identity = identity_permutation;
    Permutation perm = identity_permutation;

    std::size_t ret = 0;
    do {
        auto perm_opt = compose_permutations(view, perm);
        if (!perm_opt) {
            return std::nullopt;
        }
        perm = std::move(*perm_opt);
        ret += 1;
    } while (PermutationView{perm} != identity);
    return ret;
}

static void print_all_powers(std::FILE *stream,
                             Permutation::readonly_span view) {
    const auto identity_permutation = get_identity_permutation(view.size());
    const PermutationView identity = identity_permutation;
    auto perm = identity_permutation;

    do {
        auto perm_opt = compose_permutations(view, perm);
        if (!perm_opt) {
            std::println(stderr, "error");
            break;
        }
        perm = std::move(*perm_opt);
        print_permutation_differently(stream, perm);
        std::print(stream, ",  ");
    } while (perm != identity);
}

template <concepts::bool_or_void_c ReturnTypeOfCallBack>
[[nodiscard]] static ReturnTypeOfCallBack
calc_permutation(std::function<ReturnTypeOfCallBack(PermutationView)> call_back,
                 Permutation &all_perm, Permutation::span filled,
                 Permutation::span unfilled) {

    typedef typename Permutation::uint_t uint_t;

    std::span<uint_t> all = all_perm.get_span();
    std::size_t sum = filled.size() + unfilled.size();
    assert(std::cmp_equal(sum, all.size()));
    assert(filled.empty() || (all.begin() == filled.begin()));
    assert(unfilled.empty() ||
           (all.begin() + filled.size()) == unfilled.begin());
    if (std::cmp_equal(unfilled.size(), 0)) {
        return call_back(all_perm);
    }
    uint_t i = 0;
    bool found = false;
    const uint_t last = all.size() - 1;
    while (i <= last) {
        while (i <= last) {
            found = false;
            for (const uint_t used : filled) {
                if (i == used) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                break;
            }
            i++;
        }
        if (i > last) {
            break;
        }
        assert(!found);
        unfilled[0] = i;
        auto call_recursive = [&] {
            return calc_permutation(call_back, all_perm,
                                    all.first(filled.size() + 1),
                                    all.last(unfilled.size() - 1));
        };
        if constexpr (std::is_same_v<ReturnTypeOfCallBack, void>) {
            call_recursive();
        } else if (!call_recursive()) {
            return false;
        }

        i++;
    }
    return static_cast<ReturnTypeOfCallBack>(true);
}

template <std::size_t places> [[nodiscard]] bool print_permutation() {
    const auto max_number_of_digits = 'Z' - 'A' + 1z;
    if (std::cmp_greater(places, max_number_of_digits)) {
        return false;
    }
    std::string str(places, '\0');
    std::span all(str.data(), str.length());

    auto print = [](const Permutation &perm) -> void {
        //print_span(view);
        //print_permutation_differently(stdout, view);
        print_all_powers(stdout, perm);
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

template <concepts::range_of_PermutationView_likes_c R,
          concepts::uint32_c UInt32>
[[nodiscard]] static bool print_table(R perms, UInt32 places) {
    auto print_cell = [](PermutationView perm, std::string_view row,
                         std::string_view column) -> bool {
        bool is_header = row == "header" || column == "header";
        std::string perm_str = perm.to_string();
        const auto &hover_text = perm_str;
        auto display_text_opt = get_other_permutation_representation(perm);
        if (!display_text_opt) {
            std::println(stderr, "this is the fucked up thing: {}", perm);
            return false;
        }
        auto order_opt = get_order(perm);
        if (!order_opt) {
            return false;
        }
        static constexpr const char format[] =
            "<t{5} "
            R"~(class="{1}{2} row_{6} column_{7}" )~"
            R"~(data-row="{6}" data-column="{7}" data-perm="{1}" )~"
            R"~(title="{3}, order: {4}">)~"
            "{0}"
            "</t{5}>";
        std::print(format, *display_text_opt, perm_str,
                   (is_header ? " table_header" : ""), hover_text, *order_opt,
                   (is_header ? 'h' : 'd'), row, column);
        return true;
    };

    auto print_row = [&](PermutationView perm_row,
                         bool is_header_row = false) -> bool {
        for (PermutationView perm_column : perms) {
            auto opt =
                compose_permutations(perm_row.get_readonly_span(), perm_column);
            std::string perm_column_str = perm_column.to_string();
            std::string perm_row_str = perm_row.to_string();
            if (!opt || !print_cell(*opt,
                                    (is_header_row ? std::string_view{"header"}
                                                   : perm_row_str),
                                    perm_column_str)) {
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
    for (PermutationView perm_row : perms) {
        std::print("<tr>");
        print_cell(perm_row, perm_row.to_string(), "header");
        if (!print_row(perm_row))
            return false;
    }
    std::println("</tbody></table>");
    return true;
}

template <concepts::range_of_PermutationView_likes_c R,
          concepts::uint32_c UInt32>
[[nodiscard]] static bool print_css(R perms, UInt32 places,
                                    std::size_t number_of_permutations) {
    std::println("<style>");
    size_t i = 0;
    bool first = true;
    for (PermutationView perm : perms) {
        auto order_opt = get_order(perm);
        if (!order_opt) {
            return false;
        }
        std::size_t color_by_order = 360z * (order_opt.value() - 1z) / places;
        std::size_t color = false ? color_by_order : i;
        std::println("th.{0}:not(.selected_elm),\n"
                     "td.{0}:not(.crossed_cell) {{\n"
                     "    {1}background-color: hsl( {2}deg 75% 75% ){3};\n"
                     "}}",
                     perm.to_string(), (first ? "/*" : ""), color,
                     (first ? "*/" : ""));
        first = false;
        i += (360 / number_of_permutations);
    }
    std::println("</style>");
    std::println(R"(<link rel="stylesheet" href="./style.css" />)");
    return true;
}

template <std::ranges::range R, concepts::uint32_c UInt32>
[[nodiscard]] static bool print_table_permuted(R perms, UInt32 places) {
    assert(std::cmp_greater_equal(perms.size(), 1));
    Permutation perm(perms.size(), '\0');
    Permutation::span all(perm);

    size_t counter = 0;
    auto permute_table_and_print = [&](PermutationView view) -> bool {
        std::vector<PermutationView> new_order{};
        new_order.reserve(perms.size());
        assert(view.size() == perms.size());
        const std::size_t size = view.size();
        for (Permutation::uint_t index : view) {
            if (std::cmp_greater_equal(index, size))
                return false;
            new_order.push_back(PermutationView{perms[index]});
        }
        std::println("<br/><p>Tabelle {}, {}</p>", counter, view.to_string());
        if (!print_table(new_order, places))
            return false;
        counter++;
        return true;
    };

    return calc_permutation<bool>(permute_table_and_print, perm, all.first(0),
                                  all);
}

static bool compare_by_order(Permutation::readonly_span a,
                             Permutation::readonly_span b) {
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
    Permutation str(places);
    auto all = str.get_span();

    std::size_t number_of_permutations = fakultät(static_cast<size_t>(places));

    std::vector<Permutation> perms{};
    perms.reserve(number_of_permutations);

    auto put_into_vector = [&](PermutationView perm) -> void {
        perms.push_back(Permutation{perm});
    };

    calc_permutation<void>(put_into_vector, str, all.first(0), all);

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
        auto range_of_PermutationViews =
            perms | std::views::transform(
                        []<typename T>(T &&string) -> PermutationView {
                            return PermutationView{std::forward<T>(string)};
                        });
        auto vector_of_PermutationViews =
            range_of_PermutationViews | std::ranges::to<std::vector>();
        std::ranges::sort(vector_of_PermutationViews, compare_by_order);
        if (!print_table(vector_of_PermutationViews, places))
            return false;
        std::println("<br/><p>unsorted:</p>");
        if (!print_table(range_of_PermutationViews, places))
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

void check_expect(PermutationView a, PermutationView b,
                  PermutationView expected) {
    auto result = compose_permutations(a, b);
    if (!result)
        throw std::exception();

    auto res = result.value();
    if (res == expected)
        std::println(stderr, "{} x {} = {} (correct)", a.to_string(),
                     b.to_string(), res.to_string());
    else {
        std::println(stderr, "{} x {} = {}, expected {}", a.to_string(),
                     b.to_string(), res.to_string(), expected.to_string());
        assert(res == expected);
        throw std::exception();
    }
}

void check_expect(std::string_view a, std::string_view b,
                  std::string_view expected) {
    auto A = str_to_perm(a);
    auto B = str_to_perm(b);
    auto E = str_to_perm(expected);
    if (!A || !B || !E)
        throw std::exception(); // Conversion form string_view to Permutation
                                // didn't worked
    return check_expect(*A, *B, *E);
}

} // namespace permutations

int main() {
    namespace p = permutations;

    p::check_expect("ABC", "ABC", "ABC");
    p::check_expect("ABC", "CAB", "CAB");
    p::check_expect("CAB", "ABC", "CAB");

    std::string murks = "BCA";
    auto opt = p::str_to_perm(murks);
    if (!opt)
        throw std::exception();
    p::print_all_powers(stderr, *opt);

    if (!p::print_group_table(4)) {
        std::print(stderr, "error");
        return 1;
    }
    //print_binary_permutation(10,5); // n over k, binomal coefficient
    //print_ternary_permutation(1,1,5);

    return 0;
}
