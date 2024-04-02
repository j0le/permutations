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
#include <set>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

#include "group-interface.h"
#include "2by2matrix.h"

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
struct symetric_group;

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
    Permutation() = default;
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
    constexpr PermutationView get_perm_view() const;

    constexpr std::string to_string() const;

    constexpr operator readonly_span() const { return get_readonly_span(); }
    constexpr operator span() { return get_span(); }
    constexpr operator PermutationView() const;
    constexpr explicit operator symetric_group() const;

    constexpr std::size_t size() const { return m_span.size(); }
};

struct PermutationView : public Permutation::readonly_span {
  private:
    typedef Permutation::readonly_span base;

  public:
    // inherit ctors
    using base::base;

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

    constexpr explicit operator symetric_group() const;
};

constexpr Permutation::operator PermutationView() const {
    return PermutationView{this->get_readonly_span()};
}
constexpr PermutationView Permutation::get_perm_view() const {
    return this->operator PermutationView();
}

constexpr std::string Permutation::to_string() const {
    return this->operator PermutationView().to_string();
}
inline constexpr auto cmp_less = [](const Permutation &a,
                                    const Permutation &b) -> bool {
    if (a.size() < b.size())
        return true;
    if (a.size() > b.size())
        return false;

    assert(a.size() == b.size());
    const auto sa = a.get_readonly_span();
    const auto sb = b.get_readonly_span();

    for (std::size_t i = 0z; i < a.size(); ++i) {
        if (sa[i] < sb[i])
            return true;
        else if (sa[i] == sb[i])
            continue;
        else if (sa[i] > sb[i])
            return false;
    }
    return false;
};
typedef decltype(cmp_less) cmp_less_t;
typedef std::set<Permutation, cmp_less_t> set;

struct symetric_group{
    using element_type = Permutation;
    using element_view_type = PermutationView;
    using compare_type = cmp_less_t;
    std::size_t places{};
};
static_assert(group_config_c<symetric_group>);

constexpr Permutation::operator symetric_group() const {
    return symetric_group{.places = this->m_span.size()};
}

constexpr PermutationView::operator symetric_group() const {
    return symetric_group{.places = this->size()};
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

Permutation str_to_perm_or_throw(std::string_view view) {
    auto opt = str_to_perm(view);
    if (!opt)
        throw PermutationException();
    return *opt;
}

namespace concepts {
template <typename UINT>
concept Permutation_uint_c = std::same_as<UINT, Permutation::uint_t>;

template <typename PermLike>
concept PermutationView_like_c = std::convertible_to<PermLike, PermutationView>;

template <typename R>
concept range_of_PermutationViews_c =
    ::std::ranges::range<R> &&
    std::same_as<std::ranges::range_value_t<R>, PermutationView>;

template <typename R>
concept range_of_PermutationView_likes_c =
    ::std::ranges::range<R> &&
    PermutationView_like_c<std::ranges::range_value_t<R>>;
} //namespace concepts

template <>
std::optional<std::string>
get_other_representation<symetric_group>(const PermutationView span) {
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

} // namespace permutations

// https://fmt.dev/latest/api.html#formatting-user-defined-types
// https://en.cppreference.com/w/cpp/utility/format/formatter
// template specialization must be in global namespace
template <>
struct std::formatter<permutations::PermutationView, char> {

    unsigned repr_a : 1 = 0;
    unsigned repr_b : 1 = 0;

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext &ctx) {

        auto it = ctx.begin();
        for (; it != ctx.end(); it++) {
            char c = *it;
            switch (c) {
            case 'a':
                repr_a = true;
                break;

            case 'b':
                repr_b = true;
                break;

            case '}':
                return it;

            default:
                throw std::format_error(
                    "Invalid format args for PermutationView.");
            }
        }
        return it;
    }

    template <typename FmtContext>
    FmtContext::iterator format(const permutations::PermutationView &perm_view,
                                FmtContext &ctx) const {
        using namespace permutations;
        const std::size_t size = perm_view.size();

        bool repr_a = this->repr_a;
        bool repr_b = this->repr_b;
        if (!repr_a && !repr_b)
            repr_a = true;

        auto out = ctx.out();

        if (repr_a) {
            auto view = perm_view | std::views::transform(
                                        [size](Permutation::uint_t i) -> char {
                                            auto opt = index_to_char(i, size);
                                            if (!opt)
                                                throw PermutationException();
                                            return *opt;
                                        });
            static_assert(
                std::same_as<std::ranges::range_value_t<decltype(view)>, char>);

            out = std::ranges::copy(view, out).out;
        }
        if (repr_a && repr_b) {
            out = std::ranges::copy(std::string_view{" - "}, out).out;
        }
        if (repr_b) {
            auto other_repr_opt =
                get_other_representation<symetric_group>(perm_view);
            if (!other_repr_opt.has_value())
                throw PermutationException();
            out = std::ranges::copy(*other_repr_opt, out).out;
        }
        return out;
    }
};
static_assert(std::formattable<permutations::PermutationView, char>);

template <> struct std::formatter<permutations::Permutation, char> {

    std::formatter<permutations::PermutationView> view_formatter{};

    template <class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext &ctx) {
        return view_formatter.parse(ctx);
    }

    template <typename FmtContext>
    FmtContext::iterator format(const permutations::Permutation &perm,
                                FmtContext &ctx) const {
        return view_formatter.format(permutations::PermutationView{perm}, ctx);
    }
};
static_assert(std::formattable<permutations::Permutation, char>);

namespace permutations {

// Composition of permutations as if they are functions:
// a ∘ b
// (a∘b)(i) = a(b(i))
template<>
std::optional<typename symetric_group::element_type>
compose_permutations<symetric_group>(symetric_group::element_view_type a,
                     symetric_group::element_view_type b) {
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

template <group_config_c group_config_t>
std::optional<typename group_config_t::element_type>
compose_permutations(range_of_element_view_likes_c<group_config_t> auto &&range) {

    std::optional<typename group_config_t::element_type> opt;
    for (const auto &perm : range) {
        if (!opt.has_value()) {
            opt.emplace(perm);
            continue;
        }
        opt = compose_permutations<group_config_t>(*opt, perm);
        if (!opt.has_value()) {
            return opt;
        }
    }
    return opt;
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

static void print_span(std::span<char> span) {
    std::string_view view(span.data(), span.size());
    std::print("|{}|\n", view);
}

template<>
symetric_group::element_type get_identity<symetric_group>(symetric_group g) {
    return Permutation(g.places, true);
}

template <group_config_c gc>
std::optional<std::size_t> get_order(typename gc::element_view_type view) {
    gc config_obj = static_cast<gc>(view);
    const typename gc::element_type identity_permutation =
        get_identity<gc>(config_obj);
    const typename gc::element_view_type identity = identity_permutation;
    typename gc::element_type perm = identity_permutation;

    std::size_t ret = 0;
    do {
        auto perm_opt = compose_permutations<gc>(view, perm);
        if (!perm_opt) {
            return std::nullopt;
        }
        perm = std::move(*perm_opt);
        ret += 1;
    } while (typename gc::element_view_type{perm} != identity);
    return ret;
}

static void print_all_powers(std::FILE *stream,
                             Permutation::readonly_span view) {
    const Permutation identity_permutation =
        get_identity<symetric_group>(
            symetric_group{.places = view.size()});
    const PermutationView identity = identity_permutation;
    auto perm = identity_permutation;

    do {
        auto perm_opt = compose_permutations<symetric_group>(view, perm);
        if (!perm_opt) {
            std::println(stderr, "error");
            break;
        }
        perm = std::move(*perm_opt);
        std::print(stream, "{:ab}", perm);
        if (perm == identity)
            break;
        std::print(stream, ",  ");
    } while (true);
    std::println(stream, ".");
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
        //std::print(stdout, "{:ab}", view);
        print_all_powers(stdout, perm);
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

template <group_config_c group_config_t,
          range_of_element_view_likes_c<group_config_t> R>
[[nodiscard]] static bool print_table(R perms, group_config_t group_config) {
    using view_t = group_config_t::element_view_type;

    auto print_cell = [](view_t perm, std::string_view row,
                         std::string_view column) -> bool {
        bool is_header = row == "header" || column == "header";
        std::string perm_str = std::format("{}", perm);
        const auto &hover_text = perm_str;
        auto display_text_opt = get_other_representation<group_config_t>(perm);
        if (!display_text_opt) {
            std::println(stderr, "this is the fucked up thing: {}", perm);
            return false;
        }
        auto order_opt = get_order<group_config_t>(perm);
        if (!order_opt) {
            return false;
        }
        static constexpr const char format[] =
            "<t{5} "
            R"~(class="{1}{2} row_{6} column_{7}" )~"
            R"~(data-row="{6}" data-column="{7}" data-perm="{1}" )~"
            R"~(title="{3}, order: {4}">)~"
            "{8}{0}"
            "</t{5}>";
        std::print(format, *display_text_opt, perm_str,
                   (is_header ? " table_header" : ""), hover_text, *order_opt,
                   (is_header ? 'h' : 'd'), row, column,
                   (is_header && false ? R"(<input type="checkbox" />)" : ""));
        return true;
    };

    auto print_row = [&](view_t perm_row,
                         bool is_header_row = false) -> bool {
        for (view_t perm_column : perms) {
            auto opt =
                compose_permutations<group_config_t>(perm_row, perm_column);
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
    const auto identity = get_identity(group_config);
    if (!print_row(identity, true))
        return false;
    std::println("</thead>");

    // print bulk of the table
    std::println("<tbody>");
    for (view_t perm_row : perms) {
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
        auto order_opt = get_order<symetric_group>(perm);
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
    symetric_group group_config{.places = places};

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
        std::println("<br/><p>Tabelle {}, {}</p>", counter, view);
        if (!print_table(new_order, group_config))
            return false;
        counter++;
        return true;
    };

    return calc_permutation<bool>(permute_table_and_print, perm, all.first(0),
                                  all);
}

static bool compare_by_order(Permutation::readonly_span a,
                             Permutation::readonly_span b) {
    auto order_a_opt = get_order<symetric_group>(a);
    auto order_b_opt = get_order<symetric_group>(b);
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
                                     bool permute_table = false,
                                     bool print_html_end = true) {
    const auto max_number_of_digits = 'Z' - 'A' + 1z;
    if (std::cmp_greater(places, max_number_of_digits)) {
        return false;
    }
    Permutation str(places);
    auto all = str.get_span();
    const symetric_group group_config{.places = places};

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
        if (!print_table<symetric_group>(vector_of_PermutationViews, group_config))
            return false;
        std::println("<br/><p>unsorted:</p>");
        if (!print_table<symetric_group>(range_of_PermutationViews, group_config))
            return false;
    }

    if (print_html_end) {
        std::println("</body>\n</html>");
    }
    return true;
}

template <group_config_c group_config_t>
auto generate_subgroup_from(range_of_element_view_likes_c<group_config_t> auto
                                &&range) -> group_set<group_config_t> {

    using cmp_t = typename group_config_t::compare_type;
    using elm_t = typename group_config_t::element_type;
    using view_t = typename group_config_t::element_view_type;
    using set_t = group_set<group_config_t>;

    static constexpr auto cmp_wrapper = [](const elm_t &a,
                                           const elm_t &b) -> bool {
        bool less = cmp_t{}(a, b);
        std::println("compare {} {} {}", a, (less ? "< " : ">="), b);
        return less;
    };

    std::vector<elm_t> vec{};
    set_t x{};
    vec.append_range(range);
    x.insert_range(range);

    for (std::size_t i = 0; i < vec.size(); ++i) {
        elm_t current = vec[i];

        //std::println("----");
        //for(auto&x : vec){
        //    std::print("{}, ", x);
        //}
        //std::println("");

        for (std::size_t jj = 0; jj <= i; ++jj) {
            elm_t products[2]{};
            {
                view_t inner_current = vec[jj];
                products[0] = std::move(
                    compose_permutations<group_config_t>(current, inner_current)
                        .value());
                products[1] = std::move(
                    compose_permutations<group_config_t>(inner_current, current)
                        .value());
            }
            //size_t kkk =0;
            for (auto &p : products) {
                //std::println("{},{}: {}", i, kkk++, p);
                if (!x.contains(p)) {
                    //std::println(" - insert");
                    x.insert(p);
                    vec.push_back(std::move(p));
                } else {
                    //std::println(" - no insert");
                }
            }
        }
        //std::println("");
    }
    return x;
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
    auto result = compose_permutations<symetric_group>(a, b);
    if (!result)
        throw std::exception();

    auto res = result.value();
    if (res == expected)
        std::println(stderr, "{} x {} = {} (correct)", a, b, res);
    else {
        std::println(stderr, "{} x {} = {}, expected {}", a, b, res, expected);
        assert(res == expected);
        throw std::exception();
    }
}

void check_expect(std::string_view a, std::string_view b,
                  std::string_view expected) {
    return check_expect(str_to_perm_or_throw(a), str_to_perm_or_throw(b),
                        str_to_perm_or_throw(expected));
}

bool print_bla_group() {
    std::vector<two_by_two_matrix> generating_elements{};
    generating_elements.push_back(
        two_by_two_matrix{.cells{{false, true}, {true, false}}});
    generating_elements.push_back(
        two_by_two_matrix{.cells{{true, true}, {true, false}}});

    group_set<group_bla> set =
        generate_subgroup_from<group_bla>(generating_elements);
    return print_table<group_bla>(set, group_bla{});
}

} // namespace permutations

int main() {
    namespace p = permutations;

    p::check_expect("ABC", "ABC", "ABC");
    p::check_expect("ABC", "CAB", "CAB");
    p::check_expect("CAB", "ABC", "CAB");
    p::check_expect(           p::str_to_perm_or_throw("CAB"),
                    p::inverse(p::str_to_perm_or_throw("CAB")),
                               p::str_to_perm_or_throw("ABC"));

    std::string murks = "BCA";
    auto opt = p::str_to_perm(murks);
    if (!opt)
        throw std::exception();
    p::print_all_powers(stderr, *opt);

    auto print_elements = [](auto &&range) {
        std::println(stderr, "---------------");
        std::size_t i = 0;
        for (p::PermutationView perm : range) {
            std::println(stderr, "{: >2}: {:ab}", i++, perm);
        }
        std::println(stderr, "---------------");
    };

    auto generate_and_print_group = []<typename T>(T &&generating_elements) {
        auto group = p::generate_subgroup_from<p::symetric_group>(
            std::forward<T>(generating_elements));
        const auto &group_ref = group;
        decltype(print_elements){}(group_ref);
        return group;
    };

    bool HTML_error = false;
    if (!p::print_group_table(4, false, true)) {
        std::print(stderr, "error");
        HTML_error = true;
    }

    std::println(stderr, "");
    {
        auto rotation = p::str_to_perm_or_throw("BCDA");
        auto mirror = p::str_to_perm_or_throw("BADC");
        std::vector<p::PermutationView> generating_elements{};
        generating_elements.push_back(rotation);
        generating_elements.push_back(mirror);

        std::println(stderr,
                     "These are the generating elements:\n"
                     "- rotation: {:ab}, and\n"
                     "- mirror:   {:ab}",
                     rotation, mirror);

        std::println(stderr, "\nThis is one variant of the D4 group:");
        auto D4 = generate_and_print_group(generating_elements);

        const p::symetric_group group_config{.places = 4};
        const p::Permutation identity = p::get_identity(group_config);
        p::Permutation transformers[]{
            p::str_to_perm_or_throw("ADBC"),
            p::str_to_perm_or_throw("ACDB"),
            identity,
        };
        static constexpr const std::size_t number_of_transformers =
            sizeof(transformers) / sizeof(transformers[0]);
        static_assert(number_of_transformers == 3z);

        for (size_t i = 1; auto &t : transformers) {
            std::println(
                stderr, "transformer t{} is: {:ab} {}", i++, t,
                (p::PermutationView{t} == identity ? "  (identity)" : ""));
        }

        std::println(stderr, "\nLet us transform the group with it:");

        std::vector<p::Permutation> vecs[number_of_transformers]{};
        p::set collection;

        for (std::size_t i = 0;
             p::concepts::PermutationView_like_c auto &trans : transformers) {

            vecs[i] = D4 | std::views::transform([&](p::PermutationView view) {
                          p::PermutationView term[]{view, trans};
                          return p::compose_permutations<p::symetric_group>(term).value();
                      }) |
                      std::ranges::to<std::vector>();

            std::println(stderr, "M{0} := {{ x | d ∈ D4, x = d * t{0} }}:", i);
            print_elements(vecs[i]);
            std::println(stdout, "<br/><p>M{}</p>", i);
            if (!p::print_table(vecs[i], group_config)) {
                std::println(stderr, "error printing html table");
                HTML_error = true;
            }
            collection.insert_range(vecs[i]);
            i++;
        }

        // Print HTML table
        std::println(stdout, "<br/><p>sorted by D4</p>");
        if (!p::print_table<p::symetric_group>(vecs | std::ranges::views::join,
                            group_config)) {
            std::println(stderr, "error printing html table");
            HTML_error = true;
        }

        // vereinigung disjunkter Mengen: ⊍
        // Vereinigung von Mengen: ∪
        std::println(stderr, "M0 ⊍ M1 ⊍ M2 = S4:");
        print_elements(collection);
        if (std::cmp_not_equal(collection.size(), p::fakultät(4z))) {
            std::println(stderr, "collection is not the whole S4 group");
            return 1;
        }

        std::println(stderr,
                     "\nLet us conjugate the group D4 with the transformers:");

        auto get_conjugator = [](p::Permutation t) {
            p::Permutation inverse = p::inverse(t);
            return [t = std::move(t),
                    i = std::move(inverse)](p::PermutationView v) {
                p::PermutationView term[]{i, v, t};
                return p::compose_permutations<p::symetric_group>(term).value();
            };
        };

        for (size_t i = 0;
             p::concepts::PermutationView_like_c auto &trans : transformers) {
            const auto conjugate = get_conjugator(trans);

            auto vec = D4 | std::views::transform(conjugate) |
                       std::ranges::to<std::vector>();
            auto group = p::generate_subgroup_from<p::symetric_group>(vec);
            bool vec_is_group = vec.size() == group.size();

            std::println(stderr, "t{0}^-1 * D4 * t{0}  ({1}):", i,
                         (vec_is_group ? "is a group" : "is not a group"));
            print_elements(vec);

            std::println(stdout, "<br/><p>t{0}^-1 * D4 * t{0}  ({1}):</p>", i,
                         (vec_is_group ? "is a group" : "is not a group"));
            if (!p::print_table<p::symetric_group>(vec, group_config)) {
                std::println(stderr, "error printing html table");
                HTML_error = true;
            }
            i++;
        }

        std::println(stderr, "\nNow we only transform the generators, and "
                             "generate a new group of it.");

        struct group_with_transformer {
            p::set group{};
            p::Permutation transformer{};
        };

        auto cmp_groups = [](const group_with_transformer &a,
                             const group_with_transformer &b) {
            const auto &ga = a.group;
            const auto &gb = b.group;

            if (ga.size() < gb.size())
                return true;
            else if (ga.size() > gb.size())
                return false;

            assert(ga.size() == gb.size());
            std::size_t size = ga.size();
            auto a_end = ga.end();
            auto b_end = gb.end();
            for (auto ia = ga.begin(), ib = gb.begin();
                 ia != a_end && ib != b_end; ia++, ib++) {
                bool less = p::cmp_less(*ia, *ib);
                if (less)
                    return true;
                if (static_cast<p::PermutationView>(*ia) ==
                    static_cast<p::PermutationView>(*ib))
                    continue;
                return false;
            }
            return false;
        };

        std::multiset<group_with_transformer, decltype(cmp_groups)> groups{};

        for (const auto &t : vecs | std::ranges::views::join) {
            std::println(stderr, "Conjugate with transformer: {:ab}", t);

            const auto conjugate = get_conjugator(t);

            auto new_generators = generating_elements |
                                  std::views::transform(conjugate) |
                                  std::ranges::to<std::vector>();
            std::println(stderr, "The new generators are:");
            for (auto &g : new_generators) {
                std::println(stderr, "- {:ab}", g);
            }
            std::println(stderr, "The group generated by them is:");
            groups.insert(group_with_transformer{
                .group = generate_and_print_group(new_generators),
                .transformer = t});
        }

        std::println(stderr, "\nThe groups sorted:");
        const group_with_transformer *previous = nullptr;
        for (const group_with_transformer &g_with_t : groups) {
            bool equal_to_previous = false;
            if (previous != nullptr) {
                if (cmp_groups(*previous, g_with_t)) {
                    std::println(stderr, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<"
                                         "<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
                } else if (cmp_groups(g_with_t, *previous)) {
                    std::println(stderr, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>"
                                         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>");
                } else {
                    std::print(stderr, "= ");
                    equal_to_previous = true;
                }
            }
            std::println(stderr, "Conjugated with transformer: {:ab}",
                         g_with_t.transformer);
            if (!equal_to_previous) {
                print_elements(g_with_t.group);
            }
            previous = &g_with_t;
        }
        auto conjugate_by_t0 = get_conjugator(transformers[0]);
        // Check that conjugation is a homomorphism
        for (const auto &a : D4) {
            for (const auto &b : D4) {
                auto intermediate_result =
                    p::compose_permutations<p::symetric_group>(a, b).value();
                auto A = conjugate_by_t0(a);
                auto B = conjugate_by_t0(b);
                auto R1 = p::compose_permutations<p::symetric_group>(A, B).value();
                auto R2 = conjugate_by_t0(intermediate_result);
                if (p::PermutationView{R1} == p::PermutationView{R2})
                    std::println(stderr, "same");
                else
                    std::println(stderr, "not same");
            }
        }
    }
    std::println(stdout, "</body></html>");
    //print_binary_permutation(10,5); // n over k, binomal coefficient
    //print_ternary_permutation(1,1,5);

    if (HTML_error)
        return 1;
    return 0;
}
