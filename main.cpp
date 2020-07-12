#include "VulkanBase.h"

int main() {

	OS::Window window;
	VulkanBase r;

	if (!window.Create(L"Vulkan Example")) {
		return -1;
	}
	r.PrepareVulkan(window.GetParameters());

	if (!r.CreateSwapchain()) {
		return -1;
	}

	if (!r.CreateCommandBuffers()) {
		return -1;
	}

	if (!window.RenderingLoop(r)) {
		return -1;
	}
	return true;
}
