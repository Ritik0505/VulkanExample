/*Loading exported functions by library*/

#if !defined(VK_EXPORTED_FUNCTION)
#define VK_EXPORTED_FUNCTION( fun )
#endif

VK_EXPORTED_FUNCTION( vkGetInstanceProcAddr )

#undef VK_EXPORTED_FUNCTION

// Loading Global Level Functions required to create instance

#if !defined(VK_GLOBAL_LEVEL_FUNCTIONS)
#define VK_GLOBAL_LEVEL_FUNCTIONS( fun )
#endif

VK_GLOBAL_LEVEL_FUNCTIONS( vkCreateInstance );
VK_GLOBAL_LEVEL_FUNCTIONS( vkEnumerateInstanceLayerProperties );
VK_GLOBAL_LEVEL_FUNCTIONS( vkEnumerateInstanceExtensionProperties );

#undef VK_GLOBAL_LEVEL_FUNCTIONS

#if !defined(VK_INSTANCE_LEVEL_FUNCTIONS)
#define VK_INSTANCE_LEVEL_FUNCTIONS( fun )
#endif

VK_INSTANCE_LEVEL_FUNCTIONS( vkEnumeratePhysicalDevices )
VK_INSTANCE_LEVEL_FUNCTIONS( vkGetPhysicalDeviceProperties )
VK_INSTANCE_LEVEL_FUNCTIONS( vkGetPhysicalDeviceFeatures )
VK_INSTANCE_LEVEL_FUNCTIONS( vkGetPhysicalDeviceQueueFamilyProperties )
VK_INSTANCE_LEVEL_FUNCTIONS( vkCreateDevice )
VK_INSTANCE_LEVEL_FUNCTIONS( vkGetDeviceProcAddr )
VK_INSTANCE_LEVEL_FUNCTIONS( vkDestroyInstance )

#undef VK_INSTANCE_LEVEL_FUNCTIONS

#if !defined(VK_DEVICE_LEVEL_FUNCTIONS)
#define VK_DEVICE_LEVEL_FUNCTIONS( fun )
#endif

#undef VK_DEVICE_LEVEL_FUNCTIONS