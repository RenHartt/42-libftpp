#include <algorithm>
#include <numeric>
#include <random>
#include <cmath>

class PerlinNoise2D {
    std::vector<int> perm;
public:
    explicit PerlinNoise2D(unsigned seed = std::random_device{}()) {
        perm.resize(256);
        std::iota(perm.begin(), perm.end(), 0);
        std::mt19937 gen(seed);
        std::shuffle(perm.begin(), perm.end(), gen);
        perm.insert(perm.end(), perm.begin(), perm.end());
    }

    float sample(float x, float y) const {
        int X = static_cast<int>(std::floor(x)) & 255;
        int Y = static_cast<int>(std::floor(y)) & 255;
        
        // Relative x,y within cell
        float xf = x - std::floor(x);
        float yf = y - std::floor(y);

        // Compute fade curves
        float u = fade(xf);
        float v = fade(yf);

        // Hash coordinates of the corners
        int aa = perm[X + perm[Y]];
        int ab = perm[X + perm[Y + 1]];
        int ba = perm[X + 1 + perm[Y]];
        int bb = perm[X + 1 + perm[Y + 1]];

        // Add blended results from corners
        float x1 = lerp(u, grad(aa, xf, yf), grad(ba, xf - 1, yf));
        float x2 = lerp(u, grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1));
        return lerp(v, x1, x2);
    }

    float operator()(float x, float y) const { return sample(x,y); }
private:
    static float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static float lerp(float t, float a, float b) {
        return a + t * (b - a);
    }

    static float grad(int hash, float x, float y) {
        switch (hash & 3) {
            case 0: return  x +  y;
            case 1: return -x +  y;
            case 2: return  x -  y;
            case 3: return -x -  y;
            default: return 0.0f;
        }
    }

};