/*Loading exported functions by library*/

#if !defined(VK_EXPORTED_FUNCTION)
#define VK_EXPORTED_FUNCTION( fun )
#endif

VK_EXPORTED_FUNCTION( vkGetInstanceProcAddr )

#undef VK_EXPORTED_FUNCTION

// Loading Global Level Functions required to create instance

#if !defined(VK_GLOBAL_LEVEL_FUNCTION)
#define VK_GLOBAL_LEVEL_FUNCTION( fun )
#endif

VK_GLOBAL_LEVEL_FUNCTION( vkEnumerateInstanceLayerProperties )
VK_GLOBAL_LEVEL_FUNCTION( vkEnumerateInstanceExtensionProperties )
VK_GLOBAL_LEVEL_FUNCTION( vkCreateInstance )

#undef VK_GLOBAL_LEVEL_FUNCTION

#if !defined(VK_INSTANCE_LEVEL_FUNCTION)
#define VK_INSTANCE_LEVEL_FUNCTION( fun )
#endif

VK_INSTANCE_LEVEL_FUNCTION( vkEnumeratePhysicalDevices )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceFeatures )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceQueueFamilyProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkCreateDevice )
VK_INSTANCE_LEVEL_FUNCTION( vkGetDeviceProcAddr )
VK_INSTANCE_LEVEL_FUNCTION( vkEnumerateDeviceExtensionProperties )
VK_INSTANCE_LEVEL_FUNCTION( vkDestroyInstance )

//SWAPCHAIN EXTENSION FUNCTIONS
VK_INSTANCE_LEVEL_FUNCTION( vkDestroySurfaceKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfaceSupportKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfaceCapabilitiesKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfaceFormatsKHR )
VK_INSTANCE_LEVEL_FUNCTION( vkGetPhysicalDeviceSurfacePresentModesKHR )

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VK_INSTANCE_LEVEL_FUNCTION( vkCreateWin32SurfaceKHR )
#elif defined(VK_USE_PLATFORM_XCB_KHR)
VK_INSTANCE_LEVEL_FUNCTION( vkCreateXcbSurfaceKHR )
#endif

#undef VK_INSTANCE_LEVEL_FUNCTION

#if !defined(VK_DEVICE_LEVEL_FUNCTION)
#define VK_DEVICE_LEVEL_FUNCTION( fun )
#endif

VK_DEVICE_LEVEL_FUNCTION( vkGetDeviceQueue )
VK_DEVICE_LEVEL_FUNCTION( vkDestroyDevice )
VK_DEVICE_LEVEL_FUNCTION( vkDeviceWaitIdle )

//SwapChain extension function
VK_DEVICE_LEVEL_FUNCTION( vkCreateSwapchainKHR )
VK_DEVICE_LEVEL_FUNCTION( vkDestroySurfaceKHR )
VK_DEVICE_LEVEL_FUNCTION( vkGetSwapchainImagesKHR )
VK_DEVICE_LEVEL_FUNCTION( vkAcquireNextImageKHR )
VK_DEVICE_LEVEL_FUNCTION( vkQueuePresentKHR )
VK_DEVICE_LEVEL_FUNCTION( vkCreateSemaphore )

#undef VK_DEVICE_LEVEL_FUNCTION