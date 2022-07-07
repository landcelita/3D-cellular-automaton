#include "CA.h"

CA::CA(int length, const std::vector<int> alive_condition, float init_alive_ratio, bool isNeumannNeighborhood, bool isTorus) {
    this->length = length;
    this->alive_condition = alive_condition;
    this->init_alive_ratio = init_alive_ratio;
    this->isNeumannNeighborhood = isNeumannNeighborhood;
    this->isTorus = isTorus;

    this->field = std::vector<std::vector<std::vector<bool>>>(
        this->length, std::vector<std::vector<bool>>(
            this->length, std::vector<bool>(
                this->length, false
            )
        )
    );

    std::random_device rnd;
    std::default_random_engine eng(rnd());
    std::uniform_real_distribution<float> distr(0, 1);

    for (int i = 0; i < this->length; i++) {
        for (int j = 0; j < this->length; j++) {
            for (int k = 0; k < this->length; k++) {
                if (distr(eng) <= this->init_alive_ratio){
                    this->field[i][j][k] = true;
                } else {
                    this->field[i][j][k] = false;
                }
            }
        }   
    }
}

bool CA::isNextAliveWhenNeumann(const int fi, const int fj, const int fk) {
    int count_alive = 0;

    for(int di = -1; di <= 1; di += 2) {
        if((fi + di < 0 || fi + di >= this->length) && !isTorus) continue;
        int i = (fi + this->length + di) % this->length;
        if(this->field[i][fj][fk]) count_alive++;
    }

    for(int dj = -1; dj <= 1; dj += 2) {
        if((fj + dj < 0 || fj + dj >= this->length) && !isTorus) continue;
        int j = (fj + this->length + dj) % this->length;
        if(this->field[fi][j][fk]) count_alive++;
    }

    for(int dk = -1; dk <= 1; dk += 2) {
        if((fk + dk < 0 || fk + dk >= this->length) && !isTorus) continue;
        int k = (fk + this->length + dk) % this->length;
        if(this->field[fi][fj][k]) count_alive++;
    }

    for(const int alive_num: this->alive_condition) {
        if(count_alive == alive_num) return true;
    }

    return false;
}

bool CA::isNextAliveWhenMoore(const int fi, const int fj, const int fk) {
    int count_alive = 0;

    for(int di = -1; di <= 1; di++){
        for(int dj = -1; dj <= 1; dj++) {
            for(int dk = -1; dk <= 1; dk++) {
                if(di == 0 && dj == 0 && dk == 0) continue;
                if((fi + di < 0 || fi + di >= this->length ||
                    fj + dj < 0 || fj + dj >= this->length ||
                    fk + dk < 0 || fk + dk >= this->length) && !isTorus) {
                    continue;
                }
                int i = (fi + this->length + di) % this->length;
                int j = (fj + this->length + dj) % this->length;
                int k = (fk + this->length + dk) % this->length;
                if(this->field[i][j][k]) count_alive++;
            }
        }
    }

    for(const int alive_num: alive_condition) {
        if(count_alive == alive_num) return true;
    }

    return false;
}

void CA::progressField() {
    auto next_field = std::vector<std::vector<std::vector<bool>>>(
        this->length, std::vector<std::vector<bool>>(
            this->length, std::vector<bool>(
                this->length, false
            )
        )
    );

    for(int i = 0; i < this->length; i++) {
        for(int j = 0; j < this->length; j++) {
            for(int k = 0; k < this->length; k++) {
                if(this->isNeumannNeighborhood) {
                    next_field[i][j][k] = this->isNextAliveWhenNeumann(i, j, k);
                } else {
                    next_field[i][j][k] = this->isNextAliveWhenMoore(i, j, k);
                }
            }
        }
    }

    std::swap(field, next_field);
}

std::vector<std::vector<std::vector<bool>>> CA::getField() {
    return this->field;
};