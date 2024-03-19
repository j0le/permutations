#include <cstddef>
#include <cstdint>
#include <print>
#include <ranges>
#include <span>
#include <utility>
#include <cassert>
#include <optional>
#include <numeric>
#include <vector>
#include <functional>

static_assert(std::is_same_v<std::size_t, decltype(sizeof(0))>);
typedef std::conditional_t<std::is_signed_v<char>, signed char, unsigned char>
    underlying_char_type;

static std::optional<size_t> letter_to_index(char c, std::size_t size){
    if(c < 'A' || c > 'Z')
        return std::nullopt;

    std::size_t index = c-'A';
    if(std::cmp_greater_equal(index, size))
        return std::nullopt;
    return std::make_optional(index);
}

static std::optional<char> index_to_char(size_t index, std::size_t size){
    if(std::cmp_greater_equal(index, size))
        return std::nullopt;

    auto sum = 'A'+index;

    static_assert(std::is_same_v<decltype(index), decltype(sum)>);
    if(std::cmp_less(sum, index))
        return std::nullopt; // overflow

    underlying_char_type number_z = 'Z';
    if(std::cmp_greater(sum, number_z))
        return std::nullopt;
    char c = sum;
    return c;
}

static std::optional<std::string> apply_permutations_onto_another(std::string_view a, std::string_view b){
    if(a.size() != b.size())
        return std::nullopt;
    std::size_t size = a.size();

    std::string result(size, '\0');
    for(std::size_t i = 0; i < size; ++i){
        auto new_index_opt = letter_to_index(a[i], size);
        if(!new_index_opt)
            return std::nullopt;
        auto new_index = *new_index_opt;
        result[new_index] = b[i];
    }
    return result;
}

static void print_permutation_differently(std::string_view view){
    std::print("{} - ", view);
    // Print permutations like in the book "Elementar(st)e Gruppentheorie" by Tobias Glosauer
    // Chapter 3 "Gruppen ohne Ende", Section 3.2 "Symetrische Gruppen", page 51
    assert(std::cmp_less_equal(view.size(), 'A'-'Z'+'\01'));
    auto found = std::make_unique<bool[]>(view.size());
    for(std::size_t i = 0z; i < view.size(); ++i){
        if(found[i]){
            continue;
        }
        found[i] = true;
        char current_letter = 'A'+i;
        std::print("({}", current_letter);
        size_t next_index = i;
        while(true){
            char next_letter = view[next_index];
            if(next_letter < 'A' || next_letter > 'Z')
                throw std::exception();
            if (next_letter == current_letter)
                break;
            next_index = next_letter-'A';
            if(next_index >= view.size())
                throw std::exception();
            found[next_index] = true;
            std::print("{}", next_letter);
        }
        std::print(")");
    }
}
static void print_permutation_differently(std::span<char> span){
    std::string_view view(span.data(), span.size());
    print_permutation_differently(view);
}

static void print_span(std::span<char> span){
    std::string_view view(span.data(), span.size());
    std::print("|{}|\n",view);
}

static std::string get_identity_permutation(std::size_t places){
    std::string str(places, '\0');
    str.assign_range(std::ranges::iota_view{'A'} | std::views::take(places));
    return str;
}

static void print_all_powers(std::string_view view){
    const std::string identity_permutation = get_identity_permutation(view.size());
    std::string str = identity_permutation;

    do{
        auto string_opt = apply_permutations_onto_another(view,str);
        if(!string_opt){
            std::print("error");
            break;
        }
        str = std::move(*string_opt);
        print_permutation_differently(std::string_view{str});
        std::print(",  ");
    }while(str != identity_permutation);
}

static void print_all_powers(std::span<char> span){
    std::string_view view(span.data(), span.size());
    return print_all_powers(view);
}

static void calc_permutation(std::function<void(std::string_view)> call_back, std::span<char> all, std::span<char> filled, std::span<char> unfilled){
    std::size_t sum = filled.size() + unfilled.size();
    assert(std::cmp_equal(sum, all.size()));
    assert(filled.empty() || (all.begin() == filled.begin()));
    assert(unfilled.empty() || (all.begin()+filled.size())==unfilled.begin());
    if (std::cmp_equal(unfilled.size(), 0)) {
        std::string_view view(filled.data(), filled.size());
        call_back(view);
        return;
    }
    char c = 'A';
    bool found = false;
    const char last_char = static_cast<char>('A'+(sum-1));
    while(c <= last_char) {
        while (c <= last_char) {
            found = false;
            for (const char used : filled){
                if(c==used){
                    found = true;
                    break;
                }
            }
            if (!found){
                break;
            }
            c++;
        }
        if(c > last_char){
            break;
        }
        assert(!found);
        unfilled[0] = c;
        calc_permutation(call_back, all, all.first(filled.size()+1), all.last(unfilled.size()-1));
        c++;
    }
}

bool print_permutation(std::uint32_t places){
    const auto max_number_of_digits = 'Z'-'A'+1z;
    if (std::cmp_greater(places, max_number_of_digits)) {
        return false;
    }
    std::string str(places, '\0');
    std::span all(str.data(), str.length());

    auto print = [](std::string_view view)->void{
        //print_span(filled);
        //print_permutation_differently(filled);
        print_all_powers(view);
        std::println("");
    };

    calc_permutation(print, all, all.first(0), all);
    return true;
}

template<class Integer>
Integer fakultät(const Integer numb){
    Integer result = 1;
    for(Integer i = 1; i <= numb; ++i){
        result *= i;
    }
    return result;
}

bool print_group_table(std::uint32_t places){
    const auto max_number_of_digits = 'Z'-'A'+1z;
    if (std::cmp_greater(places, max_number_of_digits)) {
        return false;
    }
    std::string str(places, '\0');
    std::span all(str.data(), str.length());

    std::size_t number_of_permutations = fakultät(static_cast<size_t>(places));

    std::vector<std::string> perms{};
    perms.reserve(number_of_permutations);

    auto put_into_vector = [&](std::string_view view){
        perms.push_back(std::string{view});
    };

    calc_permutation(put_into_vector, all, all.first(0), all);

    assert(number_of_permutations == perms.size());

    std::println("<!DOCTYPE html>\n<html>\n<body>");

    // Print CSS for colors
    std::println("<style>");
    std::println(".table_header{{\n"
            "font-weight: bold;\n"
            "}}");
    int i = 0;
    bool first = true;
    for(auto& perm : perms){
        std::println(".{}{{\n"
                "{}background-color: hsl( {}deg 75% 75% );\n"
                "}}",
                perm,
                (first?"//":""),
                i
            );
        first = false;
        i+=(360/number_of_permutations);
    }
    std::println("</style>");
    std::println("number of permutations: {}", number_of_permutations);

    auto print_cell = [](std::string_view perm, bool header = false) {
        std::print("<td class=\"{}{}\">{}</td>", perm, (header?" table_header":""), perm);
    };

    auto print_row = [&](std::string_view perm, bool header_row = false) -> bool{
        for(auto& perm_column : perms){
            auto opt = apply_permutations_onto_another(perm,perm_column);
            if(opt)
                print_cell(*opt, header_row);
            else
                return false;
        }
        std::println("</tr>");
        return true;
    };


    std::println("<table>");
    // print header of table
    std::print("<tr><td></td>");
    const auto identity = get_identity_permutation(places);
    if(!print_row(identity, true))
        return false;

    // print bulk of the table
    for(auto& perm_row : perms){
        std::print("<tr>");
        print_cell(perm_row, true);
        if(!print_row(std::string_view{perm_row}))
            return false;
    }
    std::println("</table>");
    std::println("</body>\n</html>");
    return true;
}

static void print_binary_permutation(std::span<char> all, std::span<char> rest, std::size_t part){
    if (rest.empty()){
        //if(!all.empty()){
            print_span(all);
        //}
        return;
    }
    assert(std::cmp_greater_equal(rest.size(), part));
    if (std::cmp_greater(part, 0z)){
        rest[0] = 'x';
        print_binary_permutation(all, rest.subspan(1z), part-1z);
    }
    if(std::cmp_greater(rest.size(), part)){
        rest[0] = '_';
        print_binary_permutation(all, rest.subspan(1z), part);
    }
}

bool print_binary_permutation(std::uint32_t places, std::uint32_t part){
    if(std::cmp_less(places, part)){
        return false;
    }
    std::string str(places, '\0');
    std::span<char> span(str.data(), str.length());
    print_binary_permutation(span, span, part);
    return true;
}

static void print_ternary_permutation(std::span<char> all, std::span<char> rest, std::size_t a, std::size_t b, std::size_t c){
    if (rest.empty()){
        print_span(all);
        return;
    }
    assert(std::cmp_equal(rest.size(), a+b+c));
    if(a > 0z){
        rest[0] = 'a';
        print_ternary_permutation(all, rest.subspan(1z), a-1z, b, c);
    }
    if(b > 0z){
        rest[0] = 'B';
        print_ternary_permutation(all, rest.subspan(1z), a, b-1z, c);
    }
    if(c > 0z){
        rest[0] = ' ';
        print_ternary_permutation(all, rest.subspan(1z), a, b, c-1z);
    }
}

bool print_ternary_permutation(std::uint32_t a, std::uint32_t b, std::uint32_t c){
    static_assert(sizeof(std::size_t) >= sizeof(std::uint32_t));
    auto ab = a + b;
    if(std::cmp_less(ab,a)){
        return false; // overflow
    }
    auto abc = ab + c;
    if(std::cmp_less(abc,ab)){
        return false; // overflow
    }
    size_t sum = abc;
    std::string str(abc, '\0');
    std::span<char> span(str.data(), str.length());
    print_ternary_permutation(span, span, a, b, c);
    return true;
}

int main(){
    //std::string murks = "BA";
    //print_all_powers(std::string_view{murks});

    //print_permutation(4);
    print_group_table(3);
    //print_binary_permutation(10,5); // n over k, binomal coefficient
    //print_ternary_permutation(1,1,5);

    return 0;
}
