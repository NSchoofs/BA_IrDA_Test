#include "HardwareSerial.h"

#define MAXBUFSIZE 2048

byte buffer[MAXBUFSIZE];

HardwareSerial uart = HardwareSerial(2);

void setup() {
	uart.begin(14400, SERIAL_8N1, 16, 17, false);
}


void loop()
{
	//1. Empfangen
	while (uart.peek() == -1) { delay(50); }

	unsigned int numOfByteReceived = 0;

	while (uart.peek() != -1) {
		buffer[numOfByteReceived] = uart.read();
		numOfByteReceived++;
		delay(1);
	}

	delay(10);

	//2. Zurücksenden
	uart.write(buffer, numOfByteReceived);

	delay(500);
	unsigned int *numPtr = &numOfByteReceived;

	uart.write((byte*)numPtr, sizeof(unsigned int));

	//for (int i = 0; i < 0x50; i++) {
	//	uart.write(i);
	//	uart.flush();
	//	delay(1);
	//	//vTaskDelay(5);
	//}
	
	
}