#pragma once

#include <random>

#define gamma 0x9E3779B97F4A7C15ULL
#define C1 0xBF58476D1CE4E5B9ULL
#define C2 0x94D049BB133111EBULL

class Random2DCoordinateGenerator {
    long long seed_;
public:
    Random2DCoordinateGenerator() : seed_(std::random_device{}()) {}

    long long seed() { return seed_; }
    long long operator()(const long long& x, const long long& y) const {
        uint64_t ux = static_cast<uint64_t>(x);
        uint64_t uy = static_cast<uint64_t>(y);
        uint64_t state = static_cast<uint64_t>(seed_) ^ (ux << 32) ^ uy;

        uint64_t mixed = splitmix64(state);

        return static_cast<long long>(mixed);
    }

private:
    static uint64_t splitmix64(uint64_t z) {
        z += gamma; // Nombre d'or * 2^64
        z = (z ^ (z >> 30)) * C1; // trust Sebastiano Vigna
        z = (z ^ (z >> 27)) * C2; // trust Sebastiano Vigna
        return z ^ (z >> 31);
    }
};