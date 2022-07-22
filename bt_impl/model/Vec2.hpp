#pragma once

#include "Stream.hpp"
#include <sstream>
#include <string>
#include <cmath>

namespace model {

template<typename Real>
inline auto realNearlyEqual(Real n1, Real n2, Real epsilon = 0.00001) noexcept
{
    return n1 > n2 - epsilon && n1 < n2 + epsilon;
}

// 2 dimensional vector.
class Vec2
{
public:
    // `x` coordinate of the vector
    double x = 0;
    // `y` coordinate of the vector
    double y = 0;

    Vec2();
    Vec2(double x, double y);

    inline auto sqrLength() const noexcept { return x * x + y * y; }

    inline auto length() const noexcept { return std::sqrt(sqrLength()); }

    inline auto &normalize() noexcept
    {
        const auto len = length();
        x /= len;
        y /= len;
        return *this;
    }

    inline auto normalize() const noexcept
    {
        const auto len = length();
        return Vec2{x / len, y / len};
    }

    // Read Vec2 from input stream
    static Vec2 readFrom(InputStream &stream);

    // Write Vec2 to output stream
    void writeTo(OutputStream &stream) const;

    // Get string representation of Vec2
    std::string toString() const;
};

inline auto operator*(const Vec2 &vector, double n)
{
    return Vec2{vector.x * n, vector.y * n};
}

inline auto operator==(const Vec2 &v1, const Vec2 &v2)
{
    return realNearlyEqual(v1.x, v2.x) && realNearlyEqual(v1.y, v2.y);
}

inline auto operator-(const Vec2 &v1, const Vec2 &v2)
{
    return Vec2{v1.x - v2.x, v1.y - v2.y};
}

inline auto operator+=(Vec2 &v1, const Vec2 &v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
    return v1;
}

inline auto rayPointOrthogonalIntersect(Vec2 rayPos, Vec2 rayDir, Vec2 point)
{
    if (realNearlyEqual(rayDir.x, 0.0))
        return Vec2{rayPos.x, point.y};

    const auto m1 = rayDir.y / rayDir.x;
    const auto c1 = rayPos.y - m1 * rayPos.x;
    const auto m2 = -1 / m1;
    const auto c2 = point.y - m2 * point.x;
    const auto x = (c2 - c1) / (m1 - m2);
    return Vec2{x, m1 * x + c1};
}

inline auto rayCircleIntersectNormalVector(Vec2 rayPos, Vec2 rayDir, Vec2 center, double radius)
{
    const auto intersect = rayPointOrthogonalIntersect(rayPos, rayDir, center);
    if (center == intersect) {
        constexpr auto anyX = 1;
        // calculate orthogonal vector using dot product
        return std::pair{true, Vec2{anyX, -(anyX + intersect.x) / intersect.y}};
    } else {
        const auto normal = center - intersect;
        return std::pair{normal.sqrLength() < radius * radius, normal};
    }
}

inline auto normalizeVelocity(const Vec2 &vec, double maxSpeed)
{
    return vec.normalize() * maxSpeed;
}

inline auto dotProduct(const Vec2 &vec1, const Vec2 &vec2)
{
    return vec1.x * vec2.x + vec1.y * vec2.y;
}

} // namespace model
