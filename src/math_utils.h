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
int32_t clampi(int32_t val, int32_t min, int32_t max);
/**
 * Clamps an unsigned integer between two bounds (inclusive).
 * @param val the integer to be clamped
 * @param min the lower bound
 * @param max the upper bound
 * @return the clamped value of val
 */
uint32_t clampui(uint32_t val, uint32_t min, uint32_t max);

#endif // MATH_UTILS_H
