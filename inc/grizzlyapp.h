#ifndef HEADER_GRIZZLYAPP
#define HEADER_GRIZZLYAPP
extern "C" {
	#include "stm32f4xx.h"
}

class GrizzlyApp {
private:

public:
	//GrizzlyApp();
	//~GrizzlyApp();

	void sysConfigRCC();
	void sysConfigSysTick();
	void sysConfigDebug(uint8_t);

	void gpioConfigEthernet();
	void gpioConfigLEDOutput();

	void initReceivers();
	void runReceivers();
	
	void sysEnablePHY();
	void sysDisablePHY();
	
	void dmaStopLEDFrameTimer();
	void dmaStartLEDFrameTimer();
	void dmaInitLEDFrameTimer();
	void dmaSendRGBFrame();
	void dmaInitializeClockFalling();
	void dmaInitializeRGBData();
	void dmaInitializeClockRising();
};

#endif /* HEADER_GRIZZLYAPP */
