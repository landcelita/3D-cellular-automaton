#ifndef CA2D_H_
#define CA2D_H_

#include <vector>
#include <random>
#include <algorithm>

class CA2D
{
private:
    int length;
    std::vector<int> birth_condition;
    std::vector<int> alive_condition;
    float init_alive_ratio;
    bool isNeumannNeighborhood;
    bool isTorus;
    std::vector<std::vector<bool>> field;
    bool isNextAliveWhenNeumann(const int fi, const int fj);
    bool isNextAliveWhenMoore(const int fi, const int fj);

public:
    CA2D(int length,
        const std::vector<int> birth_condition,
        const std::vector<int> alive_condition,
        float init_alive_ratio, 
        bool isNeumannNeighborhood, 
        bool isTorus);
    void progressField();
    std::vector<std::vector<bool>> getField();
};

#endif // CA2D_H_