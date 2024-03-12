#include <cstddef>
#include <cstdint>
#include <print>
#include <span>
#include <utility>
#include <cassert>


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
    std::print("\n");
}
static void print_permutation_differently(std::span<char> span){
    std::string_view view(span.data(), span.size());
    print_permutation_differently(view);
}

static void print_span(std::span<char> span){
    std::string_view view(span.data(), span.size());
    std::print("|{}|\n",view);
}

static void print_permutation(std::span<char> all, std::span<char> filled, std::span<char> unfilled){
    std::size_t sum = filled.size() + unfilled.size();
    assert(std::cmp_equal(sum, all.size()));
    assert(filled.empty() || (all.begin() == filled.begin()));
    assert(unfilled.empty() || (all.begin()+filled.size())==unfilled.begin());
    if (std::cmp_equal(unfilled.size(), 0)) {
        //print_span(filled);
        print_permutation_differently(filled);
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
        print_permutation(all, all.first(filled.size()+1), all.last(unfilled.size()-1));
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
    print_permutation(all, all.first(0), all);
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
    print_permutation(4);
    //print_binary_permutation(10,5); // n over k, binomal coefficient
    //print_ternary_permutation(1,1,5);
    return 0;
}
