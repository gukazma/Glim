#pragma once

#include "Context.hpp"
#include <vulkan/vulkan.hpp>

namespace glim {

void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h);
void Quit();

}   // namespace glim