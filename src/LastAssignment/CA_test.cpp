#include "CA.h"
#include <vector>
#include <iostream>

void print(const std::vector<std::vector<std::vector<bool>>>& v);

int main() {
    std::vector<int> birth_condition{1, 2};
    std::vector<int> alive_condition{2, 3, 4};
    CA ca = CA(3, birth_condition, alive_condition, 0.1, false, true); // Moore-近傍
    print(ca.getField());
    std::cout << "\nNEXT\n";
    ca.progressField();
    print(ca.getField());
    std::cout << "\n\n\n";

    CA ca2 = CA(3, birth_condition, alive_condition, 0.3, true, true); // Neumann-近傍
    print(ca2.getField());
    std::cout << "\nNEXT\n";
    ca2.progressField();
    print(ca2.getField());
}


void print(const std::vector<std::vector<std::vector<bool>>>& v) {
    for(const auto ev: v) {
        for(const auto eev: ev) {
            for(const auto eeev: eev) {
                if(eeev) std::cout << '#';
                else std::cout << '.';
            }
            std::cout << '\n';
        }
        std::cout << "=================================\n";
    }
}