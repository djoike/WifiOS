#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const int serialBufferLength = 200;
char serialBuffer[serialBufferLength];	// Serial buffer is to read from serial stream through many loops
char* serialBufferPtr = serialBuffer;
bool readEndFlag = false;				// Used to trigger other logic when the termination character has been read

void setup()
{
	Serial.begin(38400); // If this changes, it must change on the roaster as well
	flushBuffer(serialBuffer,serialBufferLength,'\0'); // Reset buffer, very important!

	attemptConnect(); // Wait until Wifi is connected
}
void loop() {

	if(Serial.available())
	{
		readFromSerial(); // In any case, we read from serial if it's available
	}
	if(readEndFlag)
	{
		initiateAction(); // If the termination character has been read, we must redirect the code flow
	}
}

void attemptConnect()
{
	bool isConnected = false;
	while(!isConnected)
	{
		WiFi.begin("tr4zsb6a", "tamdr76j");
		//WiFi.begin("tplink", "911340cf4f");	
		for(int i=0; i<20; i++){
			if(WiFi.status() == WL_CONNECTED)
			{
				isConnected = true;
				break;
			}
			else
			{
				delay(500);
			}
		}
	}
}


void initiateAction()
{

	bool validCall = false;
	bool relayResponse = false;

	const int outputBufferLength = 50;
	char outputBuffer[outputBufferLength]; // This buffer is used when we parse the serial buffer to put the data into, which we must later send to RoastIO server
	flushBuffer(outputBuffer,outputBufferLength,'\0');

	const int urlBufferLength = 200;
	char urlBuffer[urlBufferLength]; // This buffer is used to build the final URL for the web call
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
			
			relayResponse = true; // This will trigger sending the server response via Serial
			
			break;
		case '2':
			validCall = true;
			strcat(urlBuffer,"/RoastIO/GetProfile.aspx");

			relayResponse = true; // This will trigger sending the server response via Serial
			
			break;
		case '3':
			validCall = validateInputString(serialBuffer);
			if(validCall)
			{
				readInputStringToOutputBuffer(serialBuffer, outputBuffer, outputBufferLength);
			}
			strcat(urlBuffer,"/RoastIO/ReceiveCurrentTemperature.aspx?temperature=");
			strcat(urlBuffer, outputBuffer);

			relayResponse = true; // This will trigger sending the server response via Serial
			
			break;
		case '4':
			validCall = validateInputString(serialBuffer);
			if(validCall)
			{
				readInputStringToOutputBuffer(serialBuffer, outputBuffer, outputBufferLength);
			}
			
			strcat(urlBuffer,"/RoastIO/ReceiveRoastData.aspx?roastdata=");
			strcat(urlBuffer, outputBuffer);
			
			relayResponse = true; // This will trigger sending the server response via Serial
			
			break;
		case '5':
			validCall = true;

			strcat(urlBuffer,"/RoastIO/GetManualRoastTemperature.aspx");
			
			relayResponse = true; // This will trigger sending the server response via Serial
			
			break;
	}
	if (validCall)
	{
		if(WiFi.status() != WL_CONNECTED)
		{
			attemptConnect();
		}

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
			if(relayResponse)
			{
				Serial.print("0");
				Serial.print(httpCode);
				Serial.print("%");
			}
		}

		http.end();
	}
	readEndFlag = false;
	flushBuffer(serialBuffer,serialBufferLength,'\0');
	serialBufferPtr = serialBuffer;
}
void readInputStringToOutputBuffer(char inputBuffer[], char outputBuffer[], int length)
{
	char* inputBufferPtr = &inputBuffer[2]; //Move to after initial "/"
	inputBuffer[strlen(inputBuffer)-1] = '\0'; //Remove trailing "/" so we only get raw data
	strcpy(outputBuffer,inputBufferPtr);
}
bool validateInputString(char inputBuffer[])
{
	return (inputBuffer[1] == '/') && (inputBuffer[strlen(inputBuffer)-1] == '/'); // It's valid if it start
}
void readFromSerial()
{
	while(Serial.available())
	{
		char c = Serial.read();
		readEndFlag = c == '$';
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