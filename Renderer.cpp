
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

	uint32_t selectedGraphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t selectedPresentQueueFamilyIndex = UINT32_MAX;

	for (auto device : PhysicalDevices) {
		if (CheckPhysicalDeviceProperties(device, selectedGraphicsQueueFamilyIndex, selectedPresentQueueFamilyIndex)) {
			handle.physicalDevice = device;
			break;
		}
	}

	if (handle.physicalDevice == VK_NULL_HANDLE) {
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

	if (vkCreateDevice(handle.physicalDevice, &deviceCreateInfo, nullptr, &handle.device) != VK_SUCCESS) {
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

	if (vkCreateWin32SurfaceKHR(handle.instance, &surfaceCreateInfo, nullptr, &handle.presentationSurface) != VK_SUCCESS) {
		std::cout << "FAILED TO CREATE PRESENTATION SURFACE " << std::endl;
		return false;
	}
	return true;

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
	if ((surfaceCapabilities.maxImageCount > 0) && (imageCount > surfaceCapabilities.maxImageCount)){
	imageCount = surfaceCapabilities.maxImageCount;
}
 	return uint32_t(imageCount);
}

VkSurfaceFormatKHR Renderer::GetSwapChainFormat(std::vector<VkSurfaceFormatKHR>& surfaceFormats)
{
	// If the list contains only one one entry with undefined format
	// It means that there are no preferred surface formats and any one can be chosen

	if ((surfaceFormats.size() == 1) && (surfaceFormats.at(0).format == VK_FORMAT_UNDEFINED)) {
		return VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
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
	Clear();

	if (handle.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(handle.device);

		if (handle.imageAvailableSemaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(handle.device, handle.imageAvailableSemaphore, nullptr);
		}

		if (handle.renderingFinishedSemaphore != VK_NULL_HANDLE) {
			vkDestroySemaphore(handle.device, handle.renderingFinishedSemaphore, nullptr);
		}

		if (handle.swapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(handle.device, handle.swapChain, nullptr);
		}

		vkDestroyDevice(handle.device, nullptr);
	}

	if (handle.presentationSurface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(handle.instance, handle.presentationSurface, nullptr);
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

bool Renderer::CreateCommandBuffers()
{
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.pNext = nullptr;
	commandPoolCreateInfo.flags = 0;
	commandPoolCreateInfo.queueFamilyIndex = handle.presentationQueueFamilyIndex;

	if (vkCreateCommandPool(handle.device, &commandPoolCreateInfo, nullptr, &handle.presentQueueCommandPool) != VK_SUCCESS) {
		std::cout << "ERROR WHILE CREATING COMMAND POOL " << std::endl;
		return false;
	}

	uint32_t imageCount = 0;
	if (vkGetSwapchainImagesKHR(handle.device, handle.swapChain, &imageCount, nullptr) != VK_SUCCESS) {
		std::cout << "COULD NOT GET NUMBER OF SWAPCHAIN IMAGES " << std::endl;
		return false;
	}

	handle.presentQueueCommandBuffers.resize(imageCount, VK_NULL_HANDLE);
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.commandPool = handle.presentQueueCommandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = imageCount;

	if (vkAllocateCommandBuffers(handle.device, &commandBufferAllocateInfo, handle.presentQueueCommandBuffers.data()) != VK_SUCCESS) {
		std::cout << "COULD NOT ALLOCATE COMMAND BUFFERS FROM COMMAND POOL " << std::endl;
		return false;
	}

	if (!RecordCommandBuffers()) {
		std::cout << "COULD NOT RECORD COMMAND BUFFERS " << std::endl;
		return false;
	}

	return true;
}

bool Renderer::RecordCommandBuffers() {
	uint32_t imageCount = static_cast<uint32_t>(handle.presentQueueCommandBuffers.size());

	std::vector<VkImage> swapchainImages(imageCount, VK_NULL_HANDLE);
	if (vkGetSwapchainImagesKHR(handle.device, handle.swapChain, &imageCount, swapchainImages.data()) != VK_SUCCESS) {
		std::cout << "COULD NOT GET SWAPCHAIN IMAGES HANDLES " << std::endl;
		return false;
	}

	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.pNext = nullptr;
	cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBufferBeginInfo.pInheritanceInfo = nullptr;

	VkClearColorValue clearColor = { 
		{1.0f, 0.8f, 0.4f, 0.0f} 
	};

	VkImageSubresourceRange imageSubresourceRange = {};
	imageSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageSubresourceRange.baseMipLevel = 0;
	imageSubresourceRange.levelCount = 1;
	imageSubresourceRange.baseArrayLayer = 0;
	imageSubresourceRange.layerCount = 1;

	for (size_t i = 0; i < imageCount; ++i) {
		VkImageMemoryBarrier memoryBarrier_present_to_clear = {};
		memoryBarrier_present_to_clear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memoryBarrier_present_to_clear.pNext = nullptr;
		memoryBarrier_present_to_clear.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; //srcAccessMask � Types of memory operations done on the image before the barrier.
		memoryBarrier_present_to_clear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; //dstAccessMask � Types of memory operations that will take place after the barrier.
		memoryBarrier_present_to_clear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		memoryBarrier_present_to_clear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memoryBarrier_present_to_clear.srcQueueFamilyIndex = handle.presentationQueueFamilyIndex;
		memoryBarrier_present_to_clear.dstQueueFamilyIndex = handle.presentationQueueFamilyIndex;
		memoryBarrier_present_to_clear.image = swapchainImages.at(i);
		memoryBarrier_present_to_clear.subresourceRange = imageSubresourceRange;

		VkImageMemoryBarrier memoryBarrier_clear_to_present = {};
		memoryBarrier_clear_to_present.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		memoryBarrier_clear_to_present.pNext = nullptr;
		memoryBarrier_clear_to_present.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; //srcAccessMask � Types of memory operations done on the image before the barrier.
		memoryBarrier_clear_to_present.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT; //dstAccessMask � Types of memory operations that will take place after the barrier.
		memoryBarrier_clear_to_present.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		memoryBarrier_clear_to_present.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		memoryBarrier_clear_to_present.srcQueueFamilyIndex = handle.presentationQueueFamilyIndex;
		memoryBarrier_clear_to_present.dstQueueFamilyIndex = handle.presentationQueueFamilyIndex;
		memoryBarrier_clear_to_present.image = swapchainImages.at(i);
		memoryBarrier_clear_to_present.subresourceRange = imageSubresourceRange;

		vkBeginCommandBuffer(handle.presentQueueCommandBuffers.at(i), &cmdBufferBeginInfo);
		vkCmdPipelineBarrier(handle.presentQueueCommandBuffers.at(i), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &memoryBarrier_present_to_clear);

		vkCmdClearColorImage(handle.presentQueueCommandBuffers.at(i), swapchainImages.at(i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			&clearColor, 1, &imageSubresourceRange);

		vkCmdPipelineBarrier(handle.presentQueueCommandBuffers.at(i), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0, 0, nullptr, 0, nullptr, 1, &memoryBarrier_clear_to_present);

		if (vkEndCommandBuffer(handle.presentQueueCommandBuffers.at(i)) != VK_SUCCESS) {
			std::cout << "COULD NOT RECORD COMMAND BUFFER " << std::endl;
			return false;
		}
	}
	return true;
}

void Renderer::Clear() {
	if (handle.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(handle.device);
		if ((handle.presentQueueCommandBuffers.size() > 0) && (handle.presentQueueCommandBuffers.at(0) == VK_NULL_HANDLE)) {
			vkFreeCommandBuffers(handle.device, handle.presentQueueCommandPool, static_cast<uint32_t>(handle.presentQueueCommandBuffers.size())
				, handle.presentQueueCommandBuffers.data());
			handle.presentQueueCommandBuffers.clear();
		}

		if (handle.presentQueueCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(handle.device, handle.presentQueueCommandPool, nullptr);
			handle.presentQueueCommandPool = VK_NULL_HANDLE;
		}
	}
}

bool Renderer::OnWindowSizeChanged() {
	Clear();

	if (!CreateSwapchain()) {
		return false;
	}

	if (!CreateCommandBuffers()) {
		return false;
	}

	return true;
}

bool Renderer::Draw()
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(handle.device, handle.swapChain, UINT32_MAX, handle.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	switch (result)
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		return OnWindowSizeChanged();
	
	default:
		std::cout << "COULD NOT ACQUIRE SWAPCHAIN IMAGE " << std::endl;
		return false;
	}

	VkPipelineStageFlags wait_Dst_StageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &handle.imageAvailableSemaphore;
	submitInfo.pWaitDstStageMask = &wait_Dst_StageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &handle.presentQueueCommandBuffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &handle.renderingFinishedSemaphore;

	if (vkQueueSubmit(handle.presentQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		return false;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &handle.renderingFinishedSemaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &handle.swapChain;
	presentInfo.pImageIndices = &imageIndex;

//	pResults � A pointer to an array of at least swapchainCount element; this parameter is optional and can be set to null,
//	but if we provide such an array, the result of the presenting operation will be stored in each of its elements, for each swap chain respectively; 
//	a single value returned by the whole function is the same as the worst result value from all swap chains.
	presentInfo.pResults = nullptr;

	result = vkQueuePresentKHR(handle.presentQueue, &presentInfo);
	switch (result)
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		break;
	case VK_ERROR_OUT_OF_DATE_KHR:
		return OnWindowSizeChanged();

	default:
		std::cout << "COULD NOT ACQUIRE SWAPCHAIN IMAGE " << std::endl;
		return false;
	}

	return true;
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

	if (!CreateSemaphores()) {
		return false;
	}

	return true;
}
