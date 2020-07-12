#pragma once
#include "VulkanBase.h"
#include "Deleter.h"

class Renderer :
    public VulkanBase
{
public:
    bool CreateRenderPass();
    bool CreateFrameBuffers();
    bool CreatePipeline();
    bool CreateSemaphores();
    bool CreateCommandBuffers();
    bool RecordCommandBuffers();

    bool Draw() override;

private:

    AutoDeleter<VkShaderModule, PFN_vkDestroyShaderModule> CreateShaderModule(const char* filename);
    AutoDeleter<VkPipelineLayout, PFN_vkDestroyPipelineLayout> CreatePipelineLayout();

    bool CreateCommandPool(uint32_t QueuefamilyIndex, VkCommandPool* pool);
    bool AllocateCommandBuffers(VkCommandPool pool, uint32_t count, VkCommandBuffer* CommandBuffers);

};

