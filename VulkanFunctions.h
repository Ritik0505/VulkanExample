#pragma once
#define VK_NO_PROTOTYPES
#include "vulkan.h"

#define VK_EXPORTED_FUNCTION( fun ) extern PFN_##fun fun;
#define VK_GLOBAL_LEVEL_FUNCTIONS( fun ) extern PFN_##fun fun;
#define VK_INSTANCE_LEVEL_FUNCTIONS( fun ) extern PFN_##fun fun;
#define VK_DEVICE_LEVEL_FUNCTIONS( fun ) extern PFN_##fun fun;		

#include "ListofFunctions.inl"
