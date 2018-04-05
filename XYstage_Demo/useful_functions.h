
int InputPos()
{
	Serial.print("Enter the set postion (0-283): ");
	while(Serial.available() == 0) ;
	int reading = Serial.parseInt();
	Serial.println(reading);
	Serial.read();
	return reading;
}

int ReadIntfromSerial()
{
	while(Serial.available() == 0) ;
	int reading = Serial.parseInt();
	Serial.println(reading);
	while(Serial.available()!=0) Serial.read();
	return reading;
}