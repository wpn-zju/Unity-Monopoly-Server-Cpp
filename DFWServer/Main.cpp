#include "enet.h"
#include "ENetController.h"
#include "MapHelper.h"

using namespace::std;

void systemSetup() {
	MapHelper::mapSetup();

	enet_initialize();
}

void systemCleanup() {
	MapHelper::mapCleanup();

	enet_deinitialize();
}

int main() {
	systemSetup();

	ENetController controller;
	controller.start();

	atexit(systemCleanup);

	return 0;
}
