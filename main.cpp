#include "Renderer.h"

int main() {

	OS::Window window;
	Renderer r;

	if (!window.Create("Vulkan Example")) {
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
