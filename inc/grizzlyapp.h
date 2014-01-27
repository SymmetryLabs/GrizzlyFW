#ifndef HEADER_GRIZZLYAPP
#define HEADER_GRIZZLYAPP
extern "C" {
	#include "stm32f4xx.h"
}

#include "grizzly_types.h"

class GrizzlyApp {
private:
	/* LocalTime when current receiving frame's first packet arrived */
	uint32_t first_frame_packet_time;
	/* Number of frames to interpolate */
	uint32_t num_interpolations;
	/* Current frame being interpolated/interleaved */
	uint32_t current_interpolation;

	/* Pointer to last FrameGroup sent */
	FramePtr last_frame;
	/* Pointer to current FrameGroup being interpolated/sent */
	FramePtr current_frame;
	
public:
	//GrizzlyApp();
	//~GrizzlyApp();
	
	GrizzyLib::ObjectManager* objman;
	void initialize();

	void sysConfigRCC();
	void sysConfigSysTick();
	void sysConfigDebug(uint8_t);

	void gpioConfigEthernet();
	void gpioConfigLEDOutput();

	void initReceivers();
	void runReceivers();

	void pushNextWorker();
	void runWorker();
	void processDMA();
	
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
