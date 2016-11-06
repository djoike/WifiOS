#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define USE_SERIAL Serial

const int serialBufferLength = 200;
char serialBuffer[serialBufferLength];
char* serialBufferPtr = serialBuffer;
bool readEndFlag = false;

void setup()
{
	Serial.begin(115200);
	flushBuffer(serialBuffer,serialBufferLength,'\0');

	WiFi.begin("MOV", "entotrefire");

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
	}
	Serial.println("CONNECTED%");
}
void loop() {

	if(Serial.available())
	{
		readFromSerial();
	}
	if(readEndFlag)
	{
		initiateAction();
	}
}

void initiateAction()
{

	readEndFlag = false;
	bool validCall = false;
	bool relayResponse = false;

	const int outputBufferLength = 50;
	char outputBuffer[outputBufferLength];
	flushBuffer(outputBuffer,outputBufferLength,'\0');

	const int urlBufferLength = 200;
	char urlBuffer[urlBufferLength];
	flushBuffer(urlBuffer,urlBufferLength,'\0');
	strcpy(urlBuffer,"http://webinterface.il-torrefattore.dk");

	switch (serialBuffer[0]) {
		case '1':
			validCall = validateInputString(serialBuffer);
			if(validCall)
			{
				readInputStringToOutputBuffer(serialBuffer, outputBuffer, outputBufferLength);
			}
			strcat(urlBuffer,"/RoastIO/RoasterStatus.aspx?code=");
			strcat(urlBuffer, outputBuffer);
			
			relayResponse = true;
			
			break;
		case '2':
			validCall = true;
			strcat(urlBuffer,"/RoastIO/GetProfile.aspx");
			relayResponse = true;
			break;
		case '3':
			validCall = validateInputString(serialBuffer);
			if(validCall)
			{
				readInputStringToOutputBuffer(serialBuffer, outputBuffer, outputBufferLength);
			}
			strcat(urlBuffer,"/RoastIO/ReceiveCurrentTemperature.aspx?temperature=");
			strcat(urlBuffer, outputBuffer);

			break;
		case '4':
			validCall = validateInputString(serialBuffer);
			if(validCall)
			{
				readInputStringToOutputBuffer(serialBuffer, outputBuffer, outputBufferLength);
			}
			
			strcat(urlBuffer,"/RoastIO/ReceiveRoastData.aspx?roastdata=");
			strcat(urlBuffer, outputBuffer);

			break;
		case '5':
			validCall = true;

			strcat(urlBuffer,"/RoastIO/GetManualRoastTemperature.aspx");
			
			relayResponse = true;
			break;
	}
	if (validCall)
	{
		HTTPClient http;
		
		http.begin(urlBuffer);
		
		int httpCode = http.GET();
		
		if(httpCode == HTTP_CODE_OK) {
			if(relayResponse)
			{
				Serial.print(serialBuffer[0]);
				Serial.print("/");
				Serial.print(http.getString());
				Serial.print("/%");
			}
		} else {
			Serial.print("0%");
		}

		http.end();
	}
	flushBuffer(serialBuffer,serialBufferLength,'\0');
	serialBufferPtr = serialBuffer;
}
void readInputStringToOutputBuffer(char inputBuffer[], char outputBuffer[], int length)
{
	char* inputBufferPtr = &inputBuffer[2]; //Move to after initial "/"
	inputBuffer[strlen(inputBuffer)-1] = '\0'; //Remove trailing "/" so we only get raw
	strcpy(outputBuffer,inputBufferPtr);
}
bool validateInputString(char inputBuffer[])
{
	return (inputBuffer[1] == '/') && (inputBuffer[strlen(inputBuffer)-1] == '/');
}
void readFromSerial()
{
	while(Serial.available())
	{
		char c = Serial.read();
		readEndFlag = c == '%';
		if(!readEndFlag)
		{
			*serialBufferPtr = c;
			serialBufferPtr++;
		}
		
	}
}
void flushBuffer(char array[], int length, char charToInsert)
{
	memset(array, charToInsert, length);
}