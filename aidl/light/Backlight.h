// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>

namespace sprd::light {

// HwLightState uses RGB luminance, with the alpha byte ignored. The kernel
// PWM range is not necessarily 255 (the E5 driver exports max_brightness).
constexpr int brightness(uint32_t color, int maximum) {
    const uint32_t luminance =
            (77 * ((color >> 16) & 255) + 150 * ((color >> 8) & 255) + 29 * (color & 255)) >> 8;
    return maximum > 0 ? static_cast<int>(uint64_t{luminance} * maximum / 255) : 0;
}

}  // namespace sprd::light
