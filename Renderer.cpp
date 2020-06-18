
#include "Renderer.h"
#include "VulkanFunctions.h"

VulkanHandles handle;

bool Renderer::LoadVulkanLibrary() {

#if defined _WIN32
	VulkanLibrary = LoadLibrary(L"vulkan-1.dll");
	if (VulkanLibrary == nullptr) {
		std::cout << "Could not load Vulkan library!" << std::endl;
		return false;
}

#elif defined __linux__
	VulkanLibrary = dlopen("libvulkan.so.1", RTLD_NOW);
	if (VulkanLibrary == nullptr) {
		std::cout << "Could not load Vulkan library!" << std::endl;
		return false;
	}
#endif
	return true;
}

bool Renderer::LoadExportedFunctions() {
#if defined _WIN32
	#define LoadProcAddress GetProcAddress
#elif defined __linux__
	#define LoadProcAddress dlsym
#endif 

#define VK_EXPORTED_FUNCTION( fun )											\
	if( !(fun = (PFN_##fun) LoadProcAddress( VulkanLibrary, #fun )) ) {		\
		std::cout <<"**COULD NOT LOAD EXPORTED FUNCTION**" << #fun ;		\
		return false;														\
	}																		\

#include "ListofFunctions.inl"
	return true;
}

bool Renderer::LoadGlobalLevelEntryPoints(){
#define VK_GLOBAL_LEVEL_FUNCTION( fun )                                                   \
	if( !(fun = (PFN_##fun) vkGetInstanceProcAddr( nullptr, #fun )) ) {                   \
		std::cout << "COULD NOT LOAD GLOBAL LEVEL FUNCTION " << #fun << std::endl;		  \
		return false;                                                                     \
	}																					  \

#include "ListOfFunctions.inl"
	return true;
}																			

bool Renderer::LoadInstanceLevelEntryPoints()
{
#define VK_INSTANCE_LEVEL_FUNCTION( fun )											\
	if( !(fun = (PFN_##fun) vkGetInstanceProcAddr( handle.instance, #fun)) ){		\
		std::cout << "COULD NOT LOAD INSTANCE LEVEL FUNCTION " << #fun << std::endl;	\
		return false;																\
	}																				\

#include "ListofFunctions.inl"
	return true;
}

bool Renderer::LoadDeviceLevelEntryPoints()
{
#define VK_DEVICE_LEVEL_FUNCTION( fun )											\
	if( !(fun = (PFN_##fun) vkGetDeviceProcAddr(handle.device, #fun)) ){			\
		std::cout << "COULD NOT LOAD DEVICE LEVEL FUNCTION " << #fun << std::endl;	\
		return false;																\
	}																				\

#include "ListofFunctions.inl"
		return true;
}

bool Renderer::CheckExtensionAvailability(const char* extension, const std::vector<VkExtensionProperties>& availableExtensions)
{
	for (size_t i = 0; i < availableExtensions.size(); i++) {
		if (strcmp(availableExtensions.at(i).extensionName, extension) == 0) {
			return true;
		}
	}
	return false;
}

bool Renderer::CreateVulkanInstance() {

	uint32_t extensionCount = 0;
	if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
		std::cout << "ERROR OCCURED DURING INSTANCE EXTENSIONS ENUMERATION " <<std::endl;
		return false;
	}

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data()) != VK_SUCCESS) {
		std::cout << "ERROR OCCURED DURING INSTANCE EXTENSIONS ENUMERATION " << std::endl;
		return false;
	}

	std::vector<const char*> requiredExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined (VK_USE_PLATFORM_WIN32_KHR)
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined (VK_USE_PLATFORM_XCB_KHR)
		VK_KHR_XCB_SURFACE_EXTENSION_NAME
#endif
	};

	for (size_t i = 0; i < requiredExtensions.size(); ++i) {
		if (!CheckExtensionAvailability(requiredExtensions.at(i), availableExtensions)) {
			std::cout << "COULD NOT FIND INSTANCE EXTENSION NAMED \"" << requiredExtensions.at(i) << "\"!" << std::endl;
			return false;
		}
	}

	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Vulkan Example : Triangle";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "No Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);


	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.ppEnabledLayerNames = nullptr;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &handle.instance) != VK_SUCCESS) {
		std::cout << "**FAILED TO CREATE INSTANCE**" << std::endl;
		return false;
	}

	return true;
}

bool Renderer::CreateLogicalDevice()
{
	uint32_t deviceCount =  0;
	if ((vkEnumeratePhysicalDevices(handle.instance, &deviceCount, nullptr)) != VK_SUCCESS) {
		std::cout << "ERROR OCURRED DURING PHYSICAL DEVICE ENUMERATION " << std::endl;
		return false;
	}

	std::vector<VkPhysicalDevice> PhysicalDevices(deviceCount);
	if ((vkEnumeratePhysicalDevices(handle.instance, &deviceCount, PhysicalDevices.data())) != VK_SUCCESS) {
		std::cout << "ERROR OCURRED DURING PHYSICAL DEVICE ENUMERATION " << std::endl;
		return false;
	}

	VkPhysicalDevice selectedDevice = VK_NULL_HANDLE;
	uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t selectedPresentQueueFamilyIndex = UINT32_MAX;

	for (auto device : PhysicalDevices) {
		if (CheckPhysicalDeviceProperties(device, selectedGraphicsQueueFamilyIndex, selectedPresentQueueFamilyIndex)) {
			selectedDevice = device;
			break;
		}
	}

	if (selectedDevice == VK_NULL_HANDLE) {
		std::cout << "**COULD NOT SELECT PHYSICAL DEVICE OF SELECTED PROPERTIES**" << std::endl;
		return false;
	}

	std::vector<float> queuePriorites = { 1.0f };
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;
	queueCreateInfo.push_back({
	VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	nullptr,
	0,
	selectedGraphicsQueueFamilyIndex,
	static_cast<uint32_t>(queuePriorites.size()),
	queuePriorites.data(),
	});

	if (selectedGraphicsQueueFamilyIndex != selectedPresentQueueFamilyIndex) {
		queueCreateInfo.push_back({
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		selectedPresentQueueFamilyIndex,
		static_cast<uint32_t>(queuePriorites.size()),
		queuePriorites.data(),
			});
	}

	std::vector<const char*> requiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = nullptr;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();
	deviceCreateInfo.enabledLayerCount = 0;
	deviceCreateInfo.ppEnabledLayerNames = nullptr;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
	deviceCreateInfo.pEnabledFeatures = nullptr;

	if (vkCreateDevice(selectedDevice, &deviceCreateInfo, nullptr, &handle.device) != VK_SUCCESS) {
		std::cout << "**COULD NOT CREATE LOGICAL DEVICE**" << std::endl;
		return false;
	}

	handle.graphicsQueueFamilyIndex = selectedGraphicsQueueFamilyIndex;
	handle.presentationQueueFamilyIndex = selectedPresentQueueFamilyIndex;
	return true;
}

bool Renderer::CheckPhysicalDeviceProperties(VkPhysicalDevice device, uint32_t& selectedGraphicsQueuefamilyIndex, uint32_t& selectedPresentQueueFamilyIndex)
{
	uint32_t extensionCount = 0;
	if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
		std::cout << "COULD NOT ENUMERATE DEVICE EXTENSION PROPERTIES" << device << std::endl;
		return false;
	}

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data()) != VK_SUCCESS) {
		std::cout << "COULD NOT ENUMERATE DEVICE EXTENSION PROPERTIES" << device << std::endl;
		return false;
	}

	std::vector<const char*> requiredExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
	};

	for (size_t i = 0; i < requiredExtensions.size(); i++) {
		if (CheckExtensionAvailability(requiredExtensions.at(i), availableExtensions)) {
			std::cout << "PHYSICAL DEVICE" << device << "DOES NOT SUPPORT EXTENSION \"" << requiredExtensions.at(i) << std::endl;
			return false;
		}
	}

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (deviceProperties.limits.maxImageDimension2D < 4096) {
		std::cout << "Physical device " << device << " doesn't support required parameters!" << std::endl;
		return false;
	}

	uint32_t queueFamiliesCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, nullptr);
	if (!queueFamiliesCount) {
		std::cout << "Physical Device : " << device << "doesn't have any queue families " << std::endl;
		return false;
	}

	std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
	std::vector<VkBool32> queuePresentSupport(queueFamiliesCount);

	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamiliesCount, queueFamiliesProperties.data());

	uint32_t graphicsQueuefamilyIndex = UINT32_MAX;
	uint32_t presentQueueFamilyIndex = UINT32_MAX;

	for (uint32_t i = 0; i < queueFamiliesCount; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, handle.presentationSurface, &queuePresentSupport[i]);
		if ((queueFamiliesProperties[i].queueCount > 0) &&
			(queueFamiliesProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			//Selecting queue that support graphics first
			if (graphicsQueuefamilyIndex == UINT32_MAX) {
				graphicsQueuefamilyIndex = i;
			}

			//Selecting Queue that supports both graphics and presentation (if there is any)
			if (queuePresentSupport[i]) {
				selectedGraphicsQueuefamilyIndex = i;
				selectedPresentQueueFamilyIndex = i;
				return true;
			}
		}
	}
	
	//If there is no queue that supports both graphics and presentation we have to select separate presentation queue
	for (uint32_t i = 0; i < queueFamiliesCount; i++) {
		if (queuePresentSupport[i]) {
			presentQueueFamilyIndex = i;
			break;
		}
	}

	if (graphicsQueuefamilyIndex == UINT32_MAX || presentQueueFamilyIndex == UINT32_MAX) {
		std::cout << "Could not find queue family with required properties on physical device " << device << "!" << std::endl;
		return false;
	}

	selectedGraphicsQueuefamilyIndex = graphicsQueuefamilyIndex;
	selectedPresentQueueFamilyIndex = presentQueueFamilyIndex;
	return true;
}

bool Renderer::GetDeviceQueue()
{
	vkGetDeviceQueue(handle.device, handle.graphicsQueueFamilyIndex, 0, &handle.graphicsQueue);
	vkGetDeviceQueue(handle.device, handle.presentationQueueFamilyIndex, 0, &handle.presentQueue);
	return true;
}

bool Renderer::CreatePresentationSurface()
{
#if defined VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.hinstance = window.Instance;
	surfaceCreateInfo.hwnd = window.Handle;

	if (vkCreateWin32SurfaceKHR(handle.instance, &surfaceCreateInfo, nullptr, &handle.presentationSurface) == VK_SUCCESS) {
		return true;
	}

#elif defined VK_USE_PLATFORM_XCB_KHR
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.flags = 0;
	surfaceCreateInfo.connection = window.Connection;
	surfaceCreateInfo.window = window.Handle;

	if (vkCreateXCBSurfaceKHR(handle.instance, &surfaceCreateInfo, nullptr, &handle.presentationSurface) == VK_SUCCESS) {
		return true;
	}
#endif

	std::cout << "COULD NOT CREATE PRESENT SURFACE" << std::endl;
	return false;
}

bool Renderer::CreateSemaphores()
{
	VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
	SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreCreateInfo.flags = 0;
	SemaphoreCreateInfo.pNext = nullptr;

	if ((vkCreateSemaphore(handle.device, &SemaphoreCreateInfo, nullptr, &handle.imageAvailableSemaphore) != VK_SUCCESS)
		|| (vkCreateSemaphore(handle.device, &SemaphoreCreateInfo, nullptr, &handle.renderingFinishedSemaphore) != VK_SUCCESS)) {
		std::cout << "COULD NOT CREATE SEMAPHORE " << std::endl;
		return false;
	}
	return false;
}

bool Renderer::CreateSwapchain()
{
	// Acquiring Surface Capabilities
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle.physicalDevice, handle.presentationSurface, &surfaceCapabilities) != VK_SUCCESS) {
		std::cout << "COULD NOT CHECK PRESENTATION SURFACE CAPABILITIES " << std::endl;
		return false;
	}

	// Acquiring Supported Surface Formats
	uint32_t formatsCount = 0;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(handle.physicalDevice, handle.presentationSurface, &formatsCount, nullptr) != VK_SUCCESS) {
		std::cout << "ERROR OCCURRED DURING PRESENTATION SURFACE FORMAT ENUMERATION " << std::endl;
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatsCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(handle.physicalDevice, handle.presentationSurface, &formatsCount, surfaceFormats.data()) != VK_SUCCESS) {
		std::cout << "ERROR OCCURRED DURING PRESENTAION SURFACE FORMAT ENUMERATION " << std::endl;
		return false;
	}

	//Acquiring Supported Presentation Modes
	uint32_t presentModesCount = 0;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(handle.physicalDevice, handle.presentationSurface, &presentModesCount, nullptr) != VK_SUCCESS) {
		std::cout << "ERROR OCCURRED DURING PRESENTATION SURFACE PRESENT MODES ENUMERATION " << std::endl;
		return false;
	}

	std::vector<VkPresentModeKHR> presentModes(presentModesCount);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(handle.physicalDevice, handle.presentationSurface, &presentModesCount, presentModes.data()) != VK_SUCCESS) {
		std::cout << "ERROR OCCURRED DURING PRESENTATION SURFACE PRESENT MODES ENUMERATION " << std::endl;
		return false;
	}

	// For SwapChain creatInfo data fields
// 	uint32_t noOfImages = GetSwapChainNumImages(surfaceCapabilities);
// 	VkSurfaceFormat

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = handle.presentationSurface;
	swapchainCreateInfo.minImageCount = GetSwapChainNumImages(surfaceCapabilities);
	
	VkSurfaceFormatKHR format = GetSwapChainFormat(surfaceFormats);
	swapchainCreateInfo.imageFormat = format.format;
	swapchainCreateInfo.imageColorSpace = format.colorSpace;

	swapchainCreateInfo.imageExtent = GetSwapChainExtent(surfaceCapabilities);

// 	imageArrayLayers � Defines the number of layers in a swap chain images(that is, views); 
// 	typically this value will be one but if we want to create multiview or stereo(stereoscopic 3D) images,
//	we can set it to some higher value.
	swapchainCreateInfo.imageArrayLayers = 1;

	swapchainCreateInfo.imageUsage = GetSwapChainUsageFlags(surfaceCapabilities);
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	swapchainCreateInfo.preTransform = GetSwapChainTransform(surfaceCapabilities);
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = GetSwapChainPresentMode(presentModes);
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = handle.swapChain;

	if (vkCreateSwapchainKHR(handle.device, &swapchainCreateInfo, nullptr, &handle.swapChain)) {
		std::cout << "COULD NOT CREATE SWAPCHAIN " << std::endl;
		return false;
	}

	if (handle.swapChain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(handle.device, handle.swapChain, nullptr);
	}
	
	return true;
}

uint32_t Renderer::GetSwapChainNumImages(VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if ((surfaceCapabilities.maxImageCount > 0) && (imageCount > surfaceCapabilities.minImageCount)){
	imageCount = surfaceCapabilities.maxImageCount;
}
 	return uint32_t(imageCount);
}

VkSurfaceFormatKHR Renderer::GetSwapChainFormat(std::vector<VkSurfaceFormatKHR>& surfaceFormats)
{
	// If the list contains only one one entry with undefined format
	// It means that there are no preferred surface formats and any one can be chosen

	if ((surfaceFormats.size() == 1) && (surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)) {
		return VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR });
	}

	//Else check if list contains R8 G8 B8 A8 format with Non Linear ColorSpace
	for (VkSurfaceFormatKHR& format : surfaceFormats) {
		if (format.format == VK_FORMAT_R8G8B8A8_UNORM) {
			return format;
		}
	}

	// Returning the first format from the list
	return surfaceFormats.at(0);
}

VkExtent2D Renderer::GetSwapChainExtent(VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	//If width == height == -1
	//we define the size by ourselves but it must fit within defined confines
	if (surfaceCapabilities.currentExtent.width == -1) {
		VkExtent2D swapchainExtent = { 800, 600 };
		if (swapchainExtent.width < surfaceCapabilities.minImageExtent.width) {
			swapchainExtent.width = surfaceCapabilities.minImageExtent.width;
		}
		if (swapchainExtent.height < surfaceCapabilities.minImageExtent.height) {
			swapchainExtent.height = surfaceCapabilities.minImageExtent.height;
		}
		if (swapchainExtent.width > surfaceCapabilities.maxImageExtent.width) {
			swapchainExtent.width = surfaceCapabilities.maxImageExtent.width;
		}
		if (swapchainExtent.height > surfaceCapabilities.maxImageExtent.height) {
			swapchainExtent.height = surfaceCapabilities.maxImageExtent.height;
		}
		return swapchainExtent;
	}
	return surfaceCapabilities.currentExtent;
}

VkImageUsageFlags Renderer::GetSwapChainUsageFlags(VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
		return VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}

	std::cout << "VK_IMAGE_USAGE_TRANSFER_DST_BIT not supported by SwapChain " << std::endl;
	std::cout << "Supported Usage Flags include: " << std::endl
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT ? "VK_IMAGE_USAGE_TRANSFER_SRC_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT ? "VK_IMAGE_USAGE_TRANSFER_DST_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT ? "VK_IMAGE_USAGE_SAMPLED_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT ? "VK_IMAGE_USAGE_STORAGE_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT ? "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT ? "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT\n" : "")
		<< (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT ? "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT\n" : "")
		<< std::endl;

	return static_cast<VkImageUsageFlags>(-1);
}

VkSurfaceTransformFlagBitsKHR Renderer::GetSwapChainTransform(VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	return surfaceCapabilities.currentTransform;
}

VkPresentModeKHR Renderer::GetSwapChainPresentMode(std::vector<VkPresentModeKHR>& presentModes)
{
	for (VkPresentModeKHR& mode : presentModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return mode;
		}
	}

	//This mode is always available
	for (VkPresentModeKHR& mode : presentModes) {
		if (mode == VK_PRESENT_MODE_FIFO_KHR) {
			return mode;
		}
	}

	std::cout << "EVEN FIFO_MODE NOT SUPPORTED" << std::endl;
	return static_cast<VkPresentModeKHR>(-1);
}

Renderer::~Renderer() {
	if (handle.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(handle.device);
		vkDestroyDevice(handle.device, nullptr);
	}

	if (handle.instance != VK_NULL_HANDLE) {
		vkDestroyInstance(handle.instance, nullptr);
	}

	if (VulkanLibrary) {
#if defined _WIN32
	FreeLibrary(VulkanLibrary);
#elif defined __linux__
	dlclose(VulkanLibrary);
#endif
	}
}

bool Renderer::PrepareVulkan(OS::WindowParameters parameters)
{
	window = parameters;

	if (!LoadVulkanLibrary() )
		return false;

	if (!LoadExportedFunctions()) {
		return false;
	}

	if (!LoadGlobalLevelEntryPoints()) {
		return false;
	}

	if (!CreateVulkanInstance()) {
		return false;
	}

	if (!LoadInstanceLevelEntryPoints()) {
		return false;
	}

	if (!CreatePresentationSurface()) {
		return false;
	}
	if (!CreateLogicalDevice()) {
		return false;
	}

	if (!LoadDeviceLevelEntryPoints()) {
		return false;
	}

	if (!GetDeviceQueue()) {
		return false;
	}

	return true;
}
