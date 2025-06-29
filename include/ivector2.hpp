#pragma once

#include <cmath>
#include <stdexcept>

template<typename TType>
struct IVector2 {
    TType x, y;

    IVector2<TType>(TType x = 0, TType y = 0) : x(x), y(y) {}

    IVector2<TType> operator+(const IVector2<TType>& other) {
        return { this->x + other.x, this->y + other.y };
    }

    IVector2<TType> operator-(const IVector2<TType>& other) {
        return { this->x - other.x, this->y - other.y };
    }

    IVector2<TType> operator*(const IVector2<TType>& other) {
        return { this->x * other.x, this->y * other.y };
    }

    IVector2<TType> operator/(const IVector2<TType>& other) {
        return { this->x / other.x, this->y / other.y };
    }

    bool operator==(const IVector2<TType>& other) {
        return this->x == other.x && this->y == other.y;
    }

    bool operator!=(const IVector2<TType>& other) {
        return this->x != other.x || this->y != other.y;
    }

    float length(void) const {
        return std::sqrt(std::pow(this->x, 2) + std::pow(this->y, 2));
    }

    IVector2<float> normalize(void) {
        return { this->x / this->length(), this->y / this->length() };
    }

    float dot(const IVector2<TType>& other) {
        return this->x * other.x + this->y * other.y;
    }

    IVector2<TType> cross(void) const {
        throw std::logic_error("https://en.wikipedia.org/wiki/Cross_product");
    }

};