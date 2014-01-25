#ifndef HEADER_GRIZZLYAPP
#define HEADER_GRIZZLYAPP

class GrizzlyApp {
private:

public:
	GrizzlyApp();
	~GrizzlyApp();

	void sysConfigRCC();
	void sysConfigSysTick();
	void sysConfigDebug(uint8_t);

	void gpioConfigEthernet();
	void gpioConfigLEDOutput();

	void initReceivers();
	void runReceivers();
};

#endif /* HEADER_GRIZZLYAPP */
