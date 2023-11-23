#include "Glim/Glim.hpp"
#include "Glim/Shader.hpp"

#include "shader_vert.h"
#include "shader_frag.h"
namespace glim {


void Init(const std::vector<const char*>& extensions, CreateSurfaceFunc func, int w, int h) {
    Context::Init(extensions, func);
    Context::GetInstance().InitSwapchain(w, h);
    Shader::Init(shader_vert,
                 sizeof(shader_vert),
                 shader_frag,
                 sizeof(shader_frag));
}

void Quit()
{
    Context::GetInstance().DestroySwapchain();
    Shader::Quit();
    Context::Quit();
}

}   // namespace glim