#pragma once
#include"group-interface.h"


namespace permutations{

struct two_by_two_matrix{
    bool cells
        [2] // row
        [2] // column
        {};
};

struct group_bla{
    using element_type = two_by_two_matrix;
    using element_view_type = two_by_two_matrix;
};
static_assert(group_config_c<group_bla>);

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
}
