#pragma once

#include <cmath>

template<typename TType>
struct IVector3 {
    TType x, y, z;

    IVector3<TType>(TType x = 0, TType y = 0, TType z = 0) : x(x), y(y), z(z) {}

    IVector3<TType> operator+(const IVector3<TType>& other) {
        return { this->x + other.x, this->y + other.y, this->z + other.z };
    }

    IVector3<TType> operator-(const IVector3<TType>& other) {
        return { this->x - other.x, this->y - other.y, this->z - other.z };
    }

    IVector3<TType> operator*(const IVector3<TType>& other) {
        return { this->x * other.x, this->y * other.y , this->z * other.z};
    }

    IVector3<TType> operator/(const IVector3<TType>& other) {
        return { this->x / other.x, this->y / other.y, this->z / other.z };
    }

    bool operator==(const IVector3<TType>& other) {
        return this->x == other.x && this->y == other.y && this->z == other.z;
    }

    bool operator!=(const IVector3<TType>& other) {
        return this->x != other.x || this->y != other.y || this->z != other.z;
    }

    float length(void) const {
        return std::sqrt(std::pow(this->x, 2) + std::pow(this->y, 2) + std::pow(this->z, 2));
    }

    IVector3<float> normalize(void) {
        return { this->x / this->length(), this->y / this->length(), this->z / this->length() };
    }

    float dot(const IVector3<TType>& other) {
        return this->x * other.x + this->y * other.y + this->z * other.z;
    }

    IVector3<TType> cross(const IVector3& other) {
        return { this->y * other.z - this->z * other.y,
                 this->z * other.x - this->x * other.z,
                 this->x * other.y - this->y * other.x };
    }

};