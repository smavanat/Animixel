/**
 * @file math_utils.h
 * @brief Contains the definition various mathematical helper functions/values
 */
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <stdint.h>

/**
 * Clamps a signed integer between two bounds (inclusive).
 * @param val the integer to be clamped
 * @param min the lower bound
 * @param max the upper bound
 * @return the clamped value of val
 */
static inline int32_t clampi(int32_t val, int32_t min, int32_t max) {
    if(val < min) return min;
    if(val > max) return max;
    return val;
}
/**
 * Clamps an unsigned integer between two bounds (inclusive).
 * @param val the integer to be clamped
 * @param min the lower bound
 * @param max the upper bound
 * @return the clamped value of val
 */
static inline uint32_t clampui(uint32_t val, uint32_t min, uint32_t max) {
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

/**
 * Returns the max between two integers a and b
 */
static inline int32_t maxi(int32_t a, int32_t b) {
    return (a > b) ? a : b;
}
/**
 * Returns the max between two unsigned integers a and b
 */
static inline uint32_t maxui(uint32_t a, uint32_t b) {
    return (a > b) ? a : b;
}
/**
 * Returns the min between two integers a and b
 */
static inline int32_t mini(int32_t a, int32_t b) {
    return (a < b) ? a : b;
}
/**
 * Returns the min between two unsigned integers a and b
 */
static inline uint32_t minui(uint32_t a, uint32_t b) {
    return (a < b) ? a : b;
}

#endif // MATH_UTILS_H
