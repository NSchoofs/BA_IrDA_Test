#include "HardwareSerial.h"

#define BUFFERSIZE 64
#define TESTVALUE 0xAA;

#define ERROR_MSG "[Error ah, feck...]"
#define SUCCESS_MSG "[Success Boi!]"

#define UART_RX 16
#define UART_TX 17

byte sendBuffer[BUFFERSIZE];
byte receiveBuffer[BUFFERSIZE];
bool sameValue[BUFFERSIZE];
bool differenceExists;
unsigned int numOfWrongValues = 0;

struct  myTime
{
	unsigned int sec, min, h, day;
};

myTime millisToSecMinHDay(unsigned long milliseconds);

HardwareSerial uart = HardwareSerial(2);

void setup() {
	Serial.begin(115200);
	Serial.println("Serial Started.");

	uart.begin(14400, SERIAL_8N1, UART_RX, UART_TX, false);
	Serial.println("UART started.");

	for (unsigned int i = 0; i < BUFFERSIZE; i++) {
		sendBuffer[i] = (byte)i;
		receiveBuffer[i] = 0x00;
	}

	delay(1000);
}


void loop()
{
	// --- Start new scan on button press
	Serial.println();
	Serial.println("Drueggsch nen Knop gehts los...");
	Serial.println();

	// --- Read all to make sure no old inputs are in buffer
	while (Serial.peek() != -1)
	{
		Serial.read();
	}

	// --- wait for press
	while (!Serial.available()) {
		delay(20);
	}

	//Störungen und co. die beim warten empfangen wurde flushen
	while (uart.peek() != -1) {
		uart.read();
	}
	myTime time;
	time = millisToSecMinHDay(millis());
	Serial.print("Startzeit (nach Einschalten): ");
	Serial.print(time.day);
	Serial.print(":");
	Serial.print(time.h);
	Serial.print(":");
	Serial.print(time.min);
	Serial.print(":");
	Serial.print(time.sec);
	Serial.println(" (d:h:m:s)");

	unsigned int numOfByteReceived = 0;
	//1. Message senden
	uart.write(sendBuffer, BUFFERSIZE);

	unsigned long waitForReceiveStart = millis();		//for timekeeping
	unsigned long waitForreceiveDuration = 0;

	//2. Auf Empfang warten
	while (uart.peek() == -1) {
		delay(50);
		if ((millis() - waitForReceiveStart) > 500) {
			Serial.print(ERROR_MSG);
			Serial.println(" Message sent, no answer received!");
			delay(500);
			return;
		}
	}

	//3. Antwort empfangen
	if (uart.available()) {
		waitForreceiveDuration = millis() - waitForReceiveStart;		//for timekeeping

		while (uart.peek() != -1) {
			byte read = uart.read();
			if (numOfByteReceived < BUFFERSIZE)		//ist noch Platz in receive buffer?
				receiveBuffer[numOfByteReceived] = read;
			numOfByteReceived++;
			delay(1);
		}
	}
	

	//4. Auswertung
	differenceExists = false;
	for (unsigned int i = 0; i < BUFFERSIZE; i++) {
		if (sendBuffer[i] != receiveBuffer[i]) {
			differenceExists = true;
			sameValue[i] = false;
		}
		else
		{
			sameValue[i] = true;
		}
	}

	//5. Ausgabe der Auswertung
	Serial.println("Message sent, answer received:");

	//Fehlerhafte Antwort?
	if (differenceExists) {

		//Fehler Zählen und ausgeben welche Bytes fehlerhaft sind
		Serial.print(ERROR_MSG);
		Serial.print(" Wrong values in answer detected. Values of received byte(s) ");
		for (unsigned int i = 0; i < BUFFERSIZE; i++) {
			if (!sameValue[i]) {
				Serial.print(" ");
				Serial.print(i);

				if (receiveBuffer[i] != 0x00) {
					Serial.print("(0x");
					Serial.print(sendBuffer[i] < 0x10 ? "0" : "");
					Serial.print(sendBuffer[i], HEX);
					Serial.print(" - 0x");
					Serial.print(receiveBuffer[i] < 0x10 ? "0" : "");
					Serial.print(receiveBuffer[i], HEX);		
					Serial.print(")");
				}

				Serial.print(" ");
				/*Serial.print(" is \"");
				Serial.print(receiveBuffer[i]);
				Serial.print("\" but should be \"");
				Serial.print(sendBuffer[i]);
				Serial.print("\".");*/

				numOfWrongValues++;
			}
		}
		Serial.print("differ(s) from the expected values. (");
		Serial.print(numOfWrongValues);
		Serial.println(" faults) ");

		//Sendung und receive visualisieren
		Serial.println("Index:  Send  -  Receive");
		for (unsigned int i = 0; i < BUFFERSIZE; i++) {
			Serial.print(i < 100 ? "0" : "");
			Serial.print(i < 10 ? "0" : "");
			Serial.print(i);
			Serial.print(":  ");
			Serial.print(sendBuffer[i] < 0x10 ? "0x0" : "0x");
			Serial.print(sendBuffer[i], HEX);
			Serial.print("  -  ");
			Serial.print(receiveBuffer[i] < 0x10 ? "0x0" : "0x");
			Serial.print(receiveBuffer[i], HEX);
			if (!sameValue[i])
				Serial.print(" <--- different value here");
			Serial.println();
		}
	}

	//Ausgabe wenn Antwort falsche Größe hat
	if (numOfByteReceived != BUFFERSIZE) {
		Serial.print(ERROR_MSG);
		Serial.print(" Answer has wrong size - Expected: ");
		Serial.print(BUFFERSIZE);
		Serial.print(" Received: ");
		Serial.print(numOfByteReceived);
		if (numOfWrongValues > 0) {
			Serial.print(" Correct:");
			Serial.print(BUFFERSIZE - numOfWrongValues);
		}
		Serial.println();
	}
	//Ausgabe bei korrekter Größe
	else	
	{
		Serial.print("Answer has correct size: ");
		Serial.print(BUFFERSIZE - numOfWrongValues);
		Serial.print("/");
		Serial.print(BUFFERSIZE);
		Serial.print(" correct");
		Serial.println();
	}
	
	//Zusammenfassung
	if (numOfByteReceived == BUFFERSIZE && !differenceExists){
		Serial.print(SUCCESS_MSG);
		Serial.println(" Message received is the same as message sent");
	}
	else if (numOfByteReceived == BUFFERSIZE) {
		Serial.println("Message received is of same lenghth as message sent, but some values were different");
	}

	Serial.print("Waited ");
	Serial.print(waitForreceiveDuration);
	Serial.println("ms for answer");

	//6. Zu Anfangsstatus zurücksetzen
	for (unsigned int i = 0; i < BUFFERSIZE; i++) {
		receiveBuffer[i] = 0x00;
	}
	numOfWrongValues = 0;

	Serial.println();
	delay(200);		//kurz abwarten um Mehrfachaufruf zu vermeiden
}

myTime millisToSecMinHDay(unsigned long milliseconds)
{
	myTime tempTime;
	
	tempTime.sec = 0;
	tempTime.min = 0;
	tempTime.h = 0;
	tempTime.day = 0;

	tempTime.sec = (unsigned int)milliseconds / 1000;
	if (tempTime.sec >= 60) {
		tempTime.min = (unsigned int)tempTime.sec / 60;
		tempTime.sec = (unsigned int)tempTime.sec % 60;
	}

	if (tempTime.min >= 60) {
		tempTime.h = (unsigned int)tempTime.min / 60;
		tempTime.min = (unsigned int)tempTime.min % 60;
	}

	if (tempTime.h >= 24) {
		tempTime.day = (unsigned int)tempTime.h / 24;
		tempTime.h = (unsigned int)tempTime.h % 24;
	}

	return tempTime;
}
