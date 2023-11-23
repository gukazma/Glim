#pragma once

#include "vulkan/vulkan.hpp"

namespace glim {

class Shader final
{
public:
    static void Init(const uint32_t* vertexShader, size_t vertexLength, const uint32_t* fragShader,
                     size_t fragLength);
    static void Quit();

    static Shader& GetInstance() { return *instance_; }

    vk::ShaderModule vertexModule;
    vk::ShaderModule fragmentModule;

    ~Shader();

private:
    static std::unique_ptr<Shader> instance_;

    Shader(const uint32_t* vertexShader, size_t vertexLength, const uint32_t* fragShader,
           size_t fragLength);
};

}   // namespace glim
