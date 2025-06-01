#pragma once

#include <cmath>
#include <string>

#include <SDL3/SDL.h>


const float EPSILON = 0.0001f;
const float INF = std::numeric_limits<float>::infinity();

static inline size_t mapQuadrantIndex(bool xPos, bool yPos) {
    /*
     * X+ Y+ => 0
     * X- Y+ => 1
     * X- Y- => 2
     * X+ Y- => 3
     */
    if (xPos) {
        return yPos ? 0 : 3;
    }
    else {
        return yPos ? 1 : 2;
    }
}

static inline float remap(float x, float from_x, float to_x, float from_y, float to_y) {
    // Fallback if the domain is bijectively unmappable
    if (to_x == from_x) {
        return from_y;
    }

    return (x - from_x) / (to_x - from_x) * (to_y - from_y) + from_y;
}

static inline bool betweenAngle(float theta, float from, float to) {
    // Takes the true positive modulo of `theta`
    theta = std::fmod(theta, 2.0f * M_PI);
    if (theta < 0) {
        theta += 2.0f * M_PI;
    }

    // Takes the true positive modulo of `from`
    from = std::fmod(from, 2.0f * M_PI);
    if (from < 0) {
        from += 2.0f * M_PI;
    }

    // Takes the true positive modulo of `to`
    to = std::fmod(to, 2.0f * M_PI);
    if (to < 0) {
        to += 2.0f * M_PI;
    }

    if (from <= to) {
        return from <= theta && theta <= to;
    }
    // Wrap-around case
    else {
        return theta >= from || theta <= to;
    }
}

static inline size_t nextPowerOfTwo(size_t x) {
    // Fallback case
    if (x == 0) {
        return 1;
    }

    x--;
    for (int i = 1; i < 64; i *= 2) {
        x |= x >> i;
    }
    x++;

    return x;
}

static inline char toHex(uint8_t n) {
    if (n < 10) {
        return '0' + n;
    }
    else if (n < 16) {
        return 'A' + (n - 10);
    }
    else {
        return 'X';
    }
}

static inline std::string toColorHex(const SDL_Color& color) {
    std::string hex(8, 'X');
    hex[0] = toHex(color.r / 16);
    hex[1] = toHex(color.r % 16);
    hex[2] = toHex(color.g / 16);
    hex[3] = toHex(color.g % 16);
    hex[4] = toHex(color.b / 16);
    hex[5] = toHex(color.b % 16);
    hex[6] = toHex(color.a / 16);
    hex[7] = toHex(color.a % 16);
    return hex;
}
