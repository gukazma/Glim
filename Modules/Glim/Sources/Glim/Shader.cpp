#include "Glim/shader.hpp"
#include "Glim/context.hpp"
#include <array>
namespace glim {

std::unique_ptr<Shader> Shader::instance_ = nullptr;

void Shader::Init(const uint32_t* vertexShader, size_t vertexLength, const uint32_t* fragShader,
                  size_t fragLength)
{
    instance_.reset(new Shader(vertexShader, vertexLength, fragShader, fragLength));
}

void Shader::Quit() {
    instance_.reset();
}

Shader::Shader(const uint32_t* vertexShader, size_t vertexLength, const uint32_t* fragShader,
               size_t fragLength)
{
    vk::ShaderModuleCreateInfo createInfo;
    createInfo.codeSize = vertexLength;
    createInfo.pCode    = vertexShader;

    vertexModule = Context::GetInstance().device.createShaderModule(createInfo);

    createInfo.codeSize = fragLength;
    createInfo.pCode    = fragShader;
    fragmentModule = Context::GetInstance().device.createShaderModule(createInfo);
}

Shader::~Shader() {
    auto& device = Context::GetInstance().device;
    device.destroyShaderModule(vertexModule);
    device.destroyShaderModule(fragmentModule);
}

}
