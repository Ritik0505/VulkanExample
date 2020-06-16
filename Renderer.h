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

struct VulkanHandles
{
public:
	VkInstance instance = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	uint32_t queueFamilyIndex = VK_NULL_HANDLE;
	VkQueue queue = VK_NULL_HANDLE;
	uint32_t graphicsQueueFamilyIndex = 0;
	uint32_t presentationQueueFamilyIndex = 0;
	VkSurfaceKHR presentationSurface = VK_NULL_HANDLE;
	VkSwapchainKHR swapChain = VK_NULL_HANDLE;

};

extern VulkanHandles handle;

class Renderer : public OS::ProjectBase
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
	bool CheckPhysicalDeviceProperties(VkPhysicalDevice device, uint32_t queuefamilyIndex);
	bool GetDeviceQueue();
	bool CreatePresentationSurface();


public:
	Renderer();
	~Renderer();
	bool PrepareVulkan( OS::WindowParameters parameters);

};

