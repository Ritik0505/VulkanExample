#include "Renderer.h"

int main() {
	Renderer r;
	r.LoadVulkanLibrary();
	r.LoadExportedFunctions();
	r.LoadGlobalLevelEntryPoints();

	return true;
}
