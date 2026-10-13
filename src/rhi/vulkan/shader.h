#pragma once

#include <iinf/rhi/shader.h>

#include <string>

#include <vulkan/vulkan_core.h>

#include <iinf/core/types.h>
#include <iinf/rhi/types.h>

namespace iinf::rhi::vulkan {

class VulkanShader final : public Shader {
public:
    VulkanShader();
    VulkanShader(VkDevice device, VkShaderModule handle, std::string entry_point);
    ~VulkanShader() override;

    VulkanShader(const VulkanShader&) = delete;
    VulkanShader& operator=(const VulkanShader&) = delete;
    VulkanShader(VulkanShader&& other) noexcept;
    VulkanShader& operator=(VulkanShader&& other) noexcept;

    std::string& entry_point() override { return entry_point_; }

    VkShaderModule handle() const { return handle_; }
    void set_handle(VkShaderModule handle) { handle_ = handle; }

private:
    std::string entry_point_;

    VkDevice device_;
    VkShaderModule handle_;
};

} // namespace iinf::rhi::vulkan
