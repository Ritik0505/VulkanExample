
#include "Renderer.h"
#include "VulkanFunctions.h"

bool Renderer::CreateRenderPass() {
	VkAttachmentDescription attachmentDescriptions = {};
	attachmentDescriptions.flags = 0;
	attachmentDescriptions.format = GetSwapChain().format;
	attachmentDescriptions.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescriptions.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference attachmentReferences = {};
	attachmentReferences.attachment = 0;
	attachmentReferences.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	VkSubpassDescription subpassDescriptions = {};
	subpassDescriptions.flags = 0;
	subpassDescriptions.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescriptions.inputAttachmentCount = 0;
	subpassDescriptions.pInputAttachments = nullptr;
	subpassDescriptions.colorAttachmentCount = 1;
	subpassDescriptions.pColorAttachments = &attachmentReferences;
	subpassDescriptions.pResolveAttachments = nullptr;
	subpassDescriptions.pDepthStencilAttachment = nullptr;
	subpassDescriptions.preserveAttachmentCount = 0;
	subpassDescriptions.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo rendpassCreateInfo = {};
	rendpassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rendpassCreateInfo.flags = 0;
	rendpassCreateInfo.pNext = nullptr;
	rendpassCreateInfo.attachmentCount = 1;
	rendpassCreateInfo.pAttachments = &attachmentDescriptions;
	rendpassCreateInfo.subpassCount = 1;
	rendpassCreateInfo.pSubpasses = &subpassDescriptions;
	rendpassCreateInfo.dependencyCount = 0;
	rendpassCreateInfo.pDependencies = nullptr;

	if (vkCreateRenderPass(GetDevice(), &rendpassCreateInfo, nullptr, &handle.renderPass) != VK_SUCCESS) {
		std::cout << "COULD NOT CREATE RENDER PASS " << std::endl;
	}

}

bool Renderer::CreateFrameBuffers() {
	const std::vector<ImageParameters> swapchainImages = GetSwapChain().images;
	handle.frameBuffers.resize(swapchainImages.size());

	for (size_t i = 0; i < swapchainImages.size(); i++) {
// 		VkImageViewCreateInfo imageViewCreateInfo = {};
// 		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
// 		imageViewCreateInfo.flags = 0;
// 		imageViewCreateInfo.pNext = nullptr;
// 		imageViewCreateInfo.image = swapchainImages[i].handle;
// 		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
// 		imageViewCreateInfo.format = GetSwapChain().format;
// 		imageViewCreateInfo.components = {
// 			VK_COMPONENT_SWIZZLE_IDENTITY,	//r
// 			VK_COMPONENT_SWIZZLE_IDENTITY,	//g
// 			VK_COMPONENT_SWIZZLE_IDENTITY,	//b
// 			VK_COMPONENT_SWIZZLE_IDENTITY,	//a
// 		};
// 		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
// 		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
// 		imageViewCreateInfo.subresourceRange.levelCount = 1;
// 		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
// 		imageViewCreateInfo.subresourceRange.layerCount = 1;
// 		if (vkCreateImageView(GetDevice(), &imageViewCreateInfo, nullptr, &swapchainImages[i].view) != VK_SUCCESS) {
// 			std::cout << "COULD NOT CREATE IMAGE VIEW " << std::endl;
// 			return false;
// 		}

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;	
		frameBufferCreateInfo.flags = 0;
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.renderPass = handle.renderPass;
		frameBufferCreateInfo.attachmentCount = 1;
		frameBufferCreateInfo.pAttachments = &swapchainImages[i].view;
		frameBufferCreateInfo.width = 300;
		frameBufferCreateInfo.height = 300;
		frameBufferCreateInfo.layers = 1;

		if (vkCreateFramebuffer(GetDevice(), &frameBufferCreateInfo, nullptr, &handle.frameBuffers[i]) != VK_SUCCESS) {
			std::cout << "COULD NOT CREATE FRAME BUFFER " << std::endl;
			return false;
		}
	}

	return true;
}