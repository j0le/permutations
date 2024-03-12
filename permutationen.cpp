#include <cstddef>
#include <cstdint>
#include <print>
#include <span>
#include <utility>
#include <cassert>


static void print_span(std::span<char> s){
    std::string_view view(s.data(), s.size());
    std::print("|{}|\n",view);
}

static void print_permutation(std::span<char> all, std::span<char> filled, std::span<char> unfilled){
    std::size_t sum = filled.size() + unfilled.size();
    assert(std::cmp_equal(sum, all.size()));
    assert(filled.empty() || (all.begin() == filled.begin()));
    assert(unfilled.empty() || (all.begin()+filled.size())==unfilled.begin());
    if (std::cmp_equal(unfilled.size(), 0)) {
        print_span(filled);
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

int main(){
    //print_permutation(4);
    print_binary_permutation(10,5); // n over k, binomal coefficient
    return 0;
}
