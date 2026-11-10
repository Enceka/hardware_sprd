// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <cstdint>
#include <istream>
#include <optional>

namespace sprd::memtrack {

// UNISOC kbase gpu_memory contains page counts, not bytes. The label "tid"
// actually contains kctx->tgid; "pid" contains kctx->pid (the calling thread).
// See drivers/gpu/arm/midgard/mali_kbase_gpu_memory_debugfs.c and
// context/mali_kbase_context.c in the ums9158 kernel tree.
std::optional<int64_t> readMaliMemory(std::istream& input, int32_t pid, int64_t pageSize);

}  // namespace sprd::memtrack
