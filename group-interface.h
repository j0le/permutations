#pragma once
#include <concepts>
#include <optional>
#include <ranges>
#include <set>
#include <string>

namespace permutations{

template <typename G>
concept group_config_c =
    std::convertible_to<typename G::element_type,
                        typename G::element_view_type> &&
    std::strict_weak_order<typename G::compare_type,
                           typename G::element_view_type,
                           typename G::element_view_type> &&
    requires(const typename G::element_view_type &a,
             const typename G::element_view_type &b) {
        { a == b } -> std::same_as<bool>;
        { a != b } -> std::same_as<bool>;
        { a.to_string() } -> std::same_as<std::string>;
    };

template <typename R, typename group_config>
concept range_of_element_view_likes_c =
    std::ranges::range<R> &&
    group_config_c<group_config> &&
    std::convertible_to<std::ranges::range_value_t<R>, typename group_config::element_type>;

template<group_config_c group_config_type>
typename group_config_type::element_type get_identity(group_config_type g = group_config_type{});

template<group_config_c group_config_t>
std::optional<typename group_config_t::element_type>
compose_permutations(typename group_config_t::element_view_type a,
                     typename group_config_t::element_view_type b);

template <group_config_c group_config_t>
std::optional<std::string> get_other_permutation_representation(
    typename group_config_t::element_view_type);

template <group_config_c group_config_t>
using group_set = std::set<typename group_config_t::element_type,
                           typename group_config_t::compare_type>;

} // namespace permutations
