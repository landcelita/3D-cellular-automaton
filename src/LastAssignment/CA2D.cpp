#include "CA2D.h"

CA2D::CA2D(int length,
    const std::vector<int> birth_condition,
    const std::vector<int> alive_condition, 
    float init_alive_ratio, 
    bool isNeumannNeighborhood, 
    bool isTorus) {
    this->length = length;
    this->birth_condition = birth_condition;
    this->alive_condition = alive_condition;
    this->init_alive_ratio = init_alive_ratio;
    this->isNeumannNeighborhood = isNeumannNeighborhood;
    this->isTorus = isTorus;

    this->field = std::vector<std::vector<bool>>(
        this->length, std::vector<bool>(
            this->length, false
        )
    );

    std::random_device rnd;
    std::default_random_engine eng(rnd());
    std::uniform_real_distribution<float> distr(0, 1);

    for (int i = 0; i < this->length; i++) {
        for (int j = 0; j < this->length; j++) {
            if (distr(eng) <= this->init_alive_ratio){
                this->field[i][j] = true;
            } else {
                this->field[i][j] = false;
            }
        }   
    }
}

bool CA2D::isNextAliveWhenNeumann(const int fi, const int fj) {
    int count_alive = 0;

    for(int di = -1; di <= 1; di += 2) {
        if((fi + di < 0 || fi + di >= this->length) && !isTorus) continue;
        int i = (fi + this->length + di) % this->length;
        if(this->field[i][fj]) count_alive++;
    }

    for(int dj = -1; dj <= 1; dj += 2) {
        if((fj + dj < 0 || fj + dj >= this->length) && !isTorus) continue;
        int j = (fj + this->length + dj) % this->length;
        if(this->field[fi][j]) count_alive++;
    }

    if (this->field[fi][fj]) {
        for(const int alive_num: this->alive_condition) {
            if(count_alive == alive_num) return true;
        }
    } else {
        for(const int birth_num: this->birth_condition) {
            if(count_alive == birth_num) return true;
        }
    }

    return false;
}

bool CA2D::isNextAliveWhenMoore(const int fi, const int fj) {
    int count_alive = 0;

    for(int di = -1; di <= 1; di++){
        for(int dj = -1; dj <= 1; dj++) {
            if(di == 0 && dj == 0) continue;
            if((fi + di < 0 || fi + di >= this->length ||
                fj + dj < 0 || fj + dj >= this->length) && !isTorus) {
                continue;
            }
            int i = (fi + this->length + di) % this->length;
            int j = (fj + this->length + dj) % this->length;
            if(this->field[i][j]) count_alive++;
        }
    }

    if (this->field[fi][fj]) {
        for(const int alive_num: this->alive_condition) {
            if(count_alive == alive_num) return true;
        }
    } else {
        for(const int birth_num: this->birth_condition) {
            if(count_alive == birth_num) return true;
        }
    }

    return false;
}

void CA2D::progressField() {
    auto next_field = std::vector<std::vector<bool>>(
            this->length, std::vector<bool>(
                this->length, false
            )
        );

    for(int i = 0; i < this->length; i++) {
        for(int j = 0; j < this->length; j++) {
            if(this->isNeumannNeighborhood) {
                next_field[i][j] = this->isNextAliveWhenNeumann(i, j);
            } else {
                next_field[i][j] = this->isNextAliveWhenMoore(i, j);
            }
        }
    }

    std::swap(field, next_field);
}

std::vector<std::vector<bool>> CA2D::getField() {
    return this->field;
};