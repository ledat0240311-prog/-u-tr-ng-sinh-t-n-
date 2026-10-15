#ifndef MATH3D_H
#define MATH3D_H

#include <cmath>

namespace LastSurvivor {

struct Vector3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};

    Vector3() = default;
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    Vector3 operator+(const Vector3& o) const { return Vector3(x + o.x, y + o.y, z + o.z); }
    Vector3 operator-(const Vector3& o) const { return Vector3(x - o.x, y - o.y, z - o.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3& operator+=(const Vector3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vector3& operator-=(const Vector3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }

    float LengthSq() const { return x * x + y * y + z * z; }
    float Length() const { return std::sqrt(LengthSq()); }

    Vector3 Normalized() const {
        float len = Length();
        if (len > 0.0001f) {
            float inv = 1.0f / len;
            return Vector3(x * inv, y * inv, z * inv);
        }
        return Vector3(0.0f, 0.0f, 0.0f);
    }

    static float Distance(const Vector3& a, const Vector3& b) {
        return (a - b).Length();
    }

    static float DistanceSq(const Vector3& a, const Vector3& b) {
        return (a - b).LengthSq();
    }

    static float Dot(const Vector3& a, const Vector3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};

} // namespace LastSurvivor

#endif // MATH3D_H
