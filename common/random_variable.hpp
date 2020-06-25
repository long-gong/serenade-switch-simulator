#ifndef RANDOM_VARIABLE_HPP
#define RANDOM_VARIABLE_HPP

#include <iostream>
#include <random>

class RV {
public:
    std::mt19937_64 gen;
    RV(unsigned seed): gen(seed) {
        std::cout << "Get seed: " << seed << std::endl;
    }
/**\brief Geometric random variable generator
 *
 *  Generates (non-negative integer-valued) geometric random variable
 *  parameterized by p, i.e., P(i;p) = p(1-p)^i,
 *
 * @param  p    distribution parameter
 * @return      non-negative integer
 */
    int geometric_random(double p) {
        std::geometric_distribution<int> distribution(p);
        return distribution(gen);
    }

/**\brief Uniform integer-valued random variable generator
 *
 * Generates an uniform distributed integer-valued random variable
 * in [a, b]
 *
 * @param a    minimum potentially generated value
 * @param b    maximum potentially generated value
 * @return     integer value inside [a, b]
 */
    int random_int_(int a, int b) {
        std::uniform_int_distribution<int> distribution(a, b);
        return distribution(gen);
    }

/**\brief Uniform integer-valued random variable generator
 *
 * Generates an uniform distributed integer-valued random variable
 * in [0, n-1]
 *
 * @param n    maximum potentially generated value plus 1
 * @return     integer value inside [0, n- 1]
 */
    int random_int(int n) {
        return random_int_(0, n - 1);
    }

/**\brief Uniform real-valued random variable generator
 *
 * Generates an uniform distributed real-valued random variable
 * in [a, b]
 *
 * @param a    minimum potentially generated value
 * @param b    maximum potentially generated value
 * @return     double type value inside [a, b)
 */
    double random_real(double a, double b) {
        std::uniform_real_distribution<double> distribution(a, b);
        return distribution(gen);
    }

/**\brief Uniform real-valued random variable generator
 *
 * Generates an uniform distributed real-valued random variable
 * in [0, 1)
 *
 * @return     double type value inside [0, 1)
 */
    double random_01() {
        return random_real(0, 1);
    }
};

#endif