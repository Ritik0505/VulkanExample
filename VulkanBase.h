#pragma once
#define VK_NO_PROTOTYPES 

#if defined _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined __linux__
#define VK_USE_PLATFORM_XCB_KHR
#endif

#include "vulkan.h"
#include "OperatingSystem.h"
#include<iostream>
#include "vector"

struct QueueParameters {
	VkQueue handle;
	uint32_t index;

	QueueParameters():
	handle(VK_NULL_HANDLE),
	index(0)	{
	}
};

struct ImageParameters {
	VkImage handle;
	VkImageView view;
	VkSampler sampler;
	VkDeviceMemory deviceMemory;

	ImageParameters() :
		handle(VK_NULL_HANDLE),
		view(VK_NULL_HANDLE),
		sampler(VK_NULL_HANDLE),
		deviceMemory(VK_NULL_HANDLE) {

	}
};

struct SwapChainParameters {
	VkSwapchainKHR handle;
	VkFormat format;
	std::vector<ImageParameters> images;
	VkExtent2D extent;

	SwapChainParameters() :
		handle(VK_NULL_HANDLE),
		format(VK_FORMAT_UNDEFINED),
		images(),
		extent() {
	}
};

struct CommonParameters{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	QueueParameters graphicsQueue;
	QueueParameters presentQueue;
	VkSurfaceKHR presentationSuface;
	SwapChainParameters swapchain;

	CommonParameters() :
		instance(VK_NULL_HANDLE),
		physicalDevice(VK_NULL_HANDLE),
		device(VK_NULL_HANDLE),
		graphicsQueue(),
		presentQueue(),
		presentationSuface(VK_NULL_HANDLE),
		swapchain() {
	}
};

struct VulkanHandles
{
public:
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	uint32_t queueFamilyIndex = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	uint32_t graphicsQueueFamilyIndex = 0;
	uint32_t presentationQueueFamilyIndex = 0;
	VkSurfaceKHR presentationSurface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderingFinishedSemaphore = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> presentQueueCommandBuffers;
	VkCommandPool presentQueueCommandPool = VK_NULL_HANDLE;

	VkRenderPass renderPass;
	std::vector<VkFramebuffer> frameBuffers;
	VkPipeline graphicsPipeline;
	VkCommandPool graphicsCommandPool;
	std::vector<VkCommandBuffer> graphicsCommandBuffers;
};

extern VulkanHandles handle;

class VulkanBase : public OS::ProjectBase
{
private:
#if defined _WIN32
	HMODULE VulkanLibrary;
#endif
#if defined __linux__
	void* VulkanLibrary;
#endif

	OS::WindowParameters window;

	bool LoadVulkanLibrary();
	bool LoadExportedFunctions();
	bool LoadGlobalLevelEntryPoints();
	bool LoadInstanceLevelEntryPoints();
	bool LoadDeviceLevelEntryPoints();
	bool CheckExtensionAvailability(const char* extension, const std::vector<VkExtensionProperties>& availableExtensions);
	bool CreateVulkanInstance();
	bool CreateLogicalDevice();
	bool CheckPhysicalDeviceProperties(VkPhysicalDevice device, uint32_t& selectedGraphicsQueuefamilyIndex, uint32_t& selectedPpresentQueueFamilyIndex);
	bool GetDeviceQueue();
	bool CreatePresentationSurface();
	bool CreateSemaphores();
	bool RecordCommandBuffers();
	void Clear();

	uint32_t GetSwapChainNumImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkSurfaceFormatKHR GetSwapChainFormat(std::vector<VkSurfaceFormatKHR>& surfaceFormats);
	VkExtent2D GetSwapChainExtent(VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkImageUsageFlags GetSwapChainUsageFlags(VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkSurfaceTransformFlagBitsKHR GetSwapChainTransform(VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	VkPresentModeKHR GetSwapChainPresentMode(std::vector<VkPresentModeKHR>& presentModes);

public:
	//Renderer();
	~VulkanBase();

	VkPhysicalDevice GetPhysicalDevice() const;
	VkDevice GetDevice() const;

	const QueueParameters GetGraphicsQueue() const;
	const QueueParameters GetPresentQueue() const;

	const SwapChainParameters& GetSwapChain() const;

	bool CreateSwapchain();
	bool CreateCommandBuffers();
	bool OnWindowSizeChanged() override;
	bool Draw() override;
	bool PrepareVulkan( OS::WindowParameters parameters);

};

