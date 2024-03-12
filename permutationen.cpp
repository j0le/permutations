#include <cstddef>
#include <cstdint>
#include <print>
#include <span>
#include <utility>
#include <cassert>

static void print_permutation(std::span<char> all, std::span<char> filled, std::span<char> unfilled){
    std::size_t sum = filled.size() + unfilled.size();
    assert(std::cmp_equal(sum, all.size()));
    assert(filled.empty() || (all.begin() == filled.begin()));
    assert(unfilled.empty() || (all.begin()+filled.size())==unfilled.begin());
    if (std::cmp_equal(unfilled.size(), 0)) {
        std::string_view view(filled.data(), filled.size());
        std::print("{}\n",view);
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

int main(){
    print_permutation(3);
    return 0;
}
