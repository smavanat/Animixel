#include "math_utils.h"

int32_t clampi(int32_t val, int32_t min, int32_t max) {
    if(val < min) return min;
    if(val > max) return max;
    return val;
}

uint32_t clampui(uint32_t val, uint32_t min, uint32_t max) {
    if(val < min) return min;
    if(val > max) return max;
    return val;
}
