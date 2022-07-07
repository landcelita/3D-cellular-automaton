#ifndef CA_H_
#define CA_H_

#include <vector>
#include <random>
#include <algorithm>

class CA
{
private:
    int length;
    std::vector<int> alive_condition;
    float init_alive_ratio;
    bool isNeumannNeighborhood;
    bool isTorus;
    std::vector<std::vector<std::vector<bool>>> field;
    bool isNextAliveWhenNeumann(const int fi, const int fj, const int fk);
    bool isNextAliveWhenMoore(const int fi, const int fj, const int fk);

public:
    CA(int length, const std::vector<int> alive_condition, float init_alive_ratio, bool isNeumannNeighborhood, bool isTorus);
    void progressField();
    std::vector<std::vector<std::vector<bool>>> getField();
};

#endif // CA_H_