#pragma once
#include <format>
#include <string>

#include "group-interface.h"

namespace permutations{

struct group_bla;
struct two_by_two_matrix{
    bool cells
        [2] // row
        [2] // column
        {};

    constexpr explicit operator group_bla() const;
    constexpr bool operator==(const two_by_two_matrix &other) const {
        for (size_t i = 0; i < 2z; i++)
            for (size_t jj = 0; jj < 2z; jj++) {
                bool A = this->cells[i][jj];
                bool B = other.cells[i][jj];
                if (A == B)
                    continue;
                return false;
            }
        return true;
    }
    constexpr std::string to_string() const {
        return std::format("M{}{}{}{}",
                           static_cast<int>(cells[0][0]),
                           static_cast<int>(cells[0][1]),
                           static_cast<int>(cells[1][0]),
                           static_cast<int>(cells[1][1]));
    };
};

inline constexpr auto cmp_2by2_matrix = [](two_by_two_matrix a,
                                           two_by_two_matrix b) -> bool {
    for (size_t i = 0; i < 2z; i++)
        for (size_t jj = 0; jj < 2z; jj++) {
            bool A = a.cells[i][jj];
            bool B = b.cells[i][jj];
            if (A == B)
                continue;
            if (!A)
                return true;
            return false;
        }
    return false;
};

constexpr bool equal_by_cmp_2by2_matrix(two_by_two_matrix a,
                                        two_by_two_matrix b) {
    return !cmp_2by2_matrix(a, b) && !cmp_2by2_matrix(b, a);
}

static_assert(equal_by_cmp_2by2_matrix(
    two_by_two_matrix{.cells{{false, true}, {false, true}}},
    two_by_two_matrix{.cells{{false, true}, {false, true}}}));
static_assert(two_by_two_matrix{.cells{{false, true}, {false, true}}} ==
              two_by_two_matrix{.cells{{false, true}, {false, true}}});
static_assert(!equal_by_cmp_2by2_matrix(
    two_by_two_matrix{.cells{{false, false}, {false, true}}},
    two_by_two_matrix{.cells{{false, true}, {false, true}}}));
static_assert(
    cmp_2by2_matrix(two_by_two_matrix{.cells{{false, false}, {false, true}}},
                    two_by_two_matrix{.cells{{false, true}, {false, true}}}));

struct group_bla{
    using element_type = two_by_two_matrix;
    using element_view_type = two_by_two_matrix;
    using compare_type = decltype(cmp_2by2_matrix);
};
static_assert(group_config_c<group_bla>);

constexpr two_by_two_matrix::operator group_bla() const { return {}; }

inline constexpr const two_by_two_matrix bla_identity{ .cells = {{true, false}, {false, true}} };

template<>
constexpr typename group_bla::element_type get_identity(group_bla g) {
    return bla_identity;
}

template <>
std::optional<two_by_two_matrix>
compose_permutations<group_bla>(two_by_two_matrix a, two_by_two_matrix b) {

    static_assert(
        two_by_two_matrix{.cells{{false, true}, {false, false}}}.cells[0][1] ==
            true,
        "Check that this is the order of indexs [row][column]");
    two_by_two_matrix ret{.cells{
        {(a.cells[0][0] && b.cells[0][0]) != (a.cells[0][1] && b.cells[1][0]),
         (a.cells[0][0] && b.cells[0][1]) != (a.cells[0][1] && b.cells[1][1])},
        {(a.cells[1][0] && b.cells[0][0]) != (a.cells[1][1] && b.cells[1][0]),
         (a.cells[1][0] && b.cells[0][1]) != (a.cells[1][1] && b.cells[1][1])}
    }};
    return ret;
}
} // namespace permutations

template <> struct std::formatter<permutations::two_by_two_matrix, char> {

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
    FmtContext::iterator format(const permutations::two_by_two_matrix &m,
                                FmtContext &ctx) const {
        using namespace permutations;

        bool repr_a = this->repr_a;
        bool repr_b = this->repr_b;
        if (!repr_a && !repr_b)
            repr_a = true;

        auto out = ctx.out();

        if (repr_a) {
            out = std::format_to(out, "M{}{}{}{}",
                                 static_cast<int>(m.cells[0][0]),
                                 static_cast<int>(m.cells[0][1]),
                                 static_cast<int>(m.cells[1][0]),
                                 static_cast<int>(m.cells[1][1]));
        }
        if (repr_a && repr_b) {
            out = std::ranges::copy(std::string_view{" - "}, out).out;
        }
        if (repr_b) {
            out = std::format_to(out, "{} {}<br/>{} {}",
                                 static_cast<int>(m.cells[0][0]),
                                 static_cast<int>(m.cells[0][1]),
                                 static_cast<int>(m.cells[1][0]),
                                 static_cast<int>(m.cells[1][1]));
        }
        return out;
    }
};
static_assert(std::formattable<permutations::two_by_two_matrix, char>);

namespace permutations {

template <>
std::optional<std::string>
get_other_representation<group_bla>(two_by_two_matrix m) {
    return std::format("{:b}", m);
}

} // namespace permutations
