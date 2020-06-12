#pragma once
#define VK_NO_PROTOTYPES 

#if defined _WIN32
	#include "windows.h"
#elif defined __linux__
	#include <xcb/xcb.h>
	#include <X11/Xlib.h>
#endif

#include "vulkan.h"
#include<iostream>
#include "vector"

struct VulkanHandles
{
	VkInstance instance;
	VkDevice device;
	uint32_t queueFamilyIndex;
	VkQueue queue;
};

extern VulkanHandles handle;

class Renderer
{
private:
#if defined _WIN32
	HMODULE VulkanLibrary;
#endif
#if defined __linux__
	void* VulkanLibrary;
#endif

	bool LoadVulkanLibrary();
	bool LoadExportedFunctions();
	bool LoadGlobalLevelEntryPoints();
	bool LoadInstanceLevelEntryPoints();
	bool LoadDeviceLevelEntryPoints();
	bool CreateVulkanInstance();
	bool CreateLogicalDevice();
	bool CheckPhysicalDeviceProperties(VkPhysicalDevice device, uint32_t queuefamilyIndex);
	bool GetDeviceQueue();

public:
	~Renderer();
	bool PrepareVulkan();

};

