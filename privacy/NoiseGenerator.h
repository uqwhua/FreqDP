//
// Created by s4570405 on 28/06/2021.
//

#ifndef HIERARCHICALGRID_NOISEGENERATOR_H
#define HIERARCHICALGRID_NOISEGENERATOR_H

#include <random>

using namespace std;

class NoiseGenerator {

public:

    static int intFromUniformDistribution(int a, int b) {
        static random_device rd;       //Will be used to obtain a seed for the random number engine
        static mt19937 gen(rd());      //Standard mersenne_twister_engine seeded with rd()

        uniform_int_distribution<int> dis(a, b);
        return dis(gen);
    }

    static float floatFromUniformDistribution(float a, float b) {     // [a,b)
        static random_device rd;       //Will be used to obtain a seed for the random number engine
        static mt19937 gen(rd());      //Standard mersenne_twister_engine seeded with rd()

        uniform_real_distribution<float> dis(a, b);
        return dis(gen);
    }

    static float fromLaplaceDistribution(float epsilon, float sensitivity, float mean) {
        float lamda = sensitivity / epsilon;        // Lap(mean = 0, lamda = sensitivity/epsilon) ~ \epsilon-DP

        double randomfloat = floatFromUniformDistribution(0.0, 1.0) - 0.5;
        double randomfloat_sng = randomfloat == 0 ? 0 : (randomfloat > 0 ? 1.0 : -1.0);

        double noise = mean - lamda * randomfloat_sng * log(1 - 2 * abs(randomfloat));
        return (float) noise;
    }
};

#endif //HIERARCHICALGRID_NOISEGENERATOR_H
