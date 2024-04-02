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
            for (size_t jj = 0; jj < 2z; i++) {
                bool A = this->cells[i][jj];
                bool B = other.cells[i][jj];
                if (A == B)
                    continue;
                return false;
            }
        return false;
    }
    constexpr std::string to_string() const {
        return std::format("M{}{}{}{}", cells[0][0], cells[0][1], cells[1][0],
                           cells[1][1]);
    };
};

inline constexpr auto cmp_2by2_matrix = [](two_by_two_matrix a,
                                           two_by_two_matrix b) -> bool {
    for (size_t i = 0; i < 2z; i++)
        for (size_t jj = 0; jj < 2z; i++) {
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

    two_by_two_matrix ret{.cells{
        {(a.cells[0][0] && b.cells[0][0]) != (a.cells[0][1] && b.cells[1][0]),
         (a.cells[0][0] && b.cells[0][1]) != (a.cells[0][1] && b.cells[1][1])},
        {(a.cells[1][0] && b.cells[0][0]) != (a.cells[1][1] && b.cells[1][0]),
         (a.cells[1][0] && b.cells[0][1]) != (a.cells[1][1] && b.cells[1][1])}
    }};
    return ret;
}

template <>
std::optional<std::string>
get_other_permutation_representation<group_bla>(two_by_two_matrix m) {
    return m.to_string();
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
            out = std::format_to(out, "M{}{}{}{}", m.cells[0][0], m.cells[0][1],
                                 m.cells[1][0], m.cells[1][1]);
        }
        //if (repr_a && repr_b) {
        //    out = std::ranges::copy(std::string_view{" - "}, out).out;
        //}
        //if (repr_b) {
        //    auto other_repr_opt =
        //        get_other_permutation_representation<group_bla>(perm_view);
        //    if (!other_repr_opt.has_value())
        //        throw PermutationException();
        //    out = std::ranges::copy(*other_repr_opt, out).out;
        //}
        return out;
    }
};
static_assert(std::formattable<permutations::two_by_two_matrix, char>);
