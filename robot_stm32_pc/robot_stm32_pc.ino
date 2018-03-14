/*
 Name:		robot_stm32_pc.ino
 Created:	10.02.2018 15:04:41
 Author:	Michał Kuzemczak
*/

// the setup function runs once when you press reset or power the board
#include <Adafruit_PWMServoDriver.h>

#define MAX_LERP_POINTS 30
#define MAX_SERVOS 5

#define SRV0  0
#define SRV1  1
#define SRV2  2
#define SRV3  3
#define SRV4  4
#define SRV5  5

Adafruit_PWMServoDriver driver = Adafruit_PWMServoDriver();

void saveData(String val);
void start();
void establishContact();
void setGripper();
void movement();
void roznica();
void sendPath();

int pathAmt = 0, cntr0 = 0, cntr1 = 0, grip = 362, lastGrip = 362;
int path[MAX_LERP_POINTS][MAX_SERVOS + 1];
char val; // bajt odebrany z portu szeregowego
String liczba = ""; // ciag bajtow odebranych z portu szeregowego
bool _receivingPath = false; // flaga ustawiana, kiedy pojawi sie sygnal o przekazywaniu sciezki
bool _receivingGrip = false;
bool _lerpPointsAmtReceived = false;  // falga powiadamiajaca, ze zapisano ilosc punktow posrednich sciezki

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(12, OUTPUT);
	Serial.begin(115200); // otwarcie portu szeregowego UART
	driver.begin();
	driver.setPWMFreq(60);
	yield();

	start();
	establishContact(); // ustanowienie kontaktu
	digitalWrite(LED_BUILTIN, HIGH);

	for (int i = 0; i < MAX_LERP_POINTS; i++)
		for (int j = 0; j < MAX_SERVOS + 1; j++)
			path[i][j] = 0;
}

// the loop function runs over and over again until power down or reset
void loop() {
	/*while (Serial.available() > 0) {
		val = Serial.read();
		if (isDigit(val)) {
			liczba += val;
		} 
		else if (val == '\n') {
			saveData(liczba);
		}
		else if (val == 'P') {
			_receivingPath = true;
		}
		else if (val == 'G') {
			_receivingGrip = true;
		}
	}*/
}

void serialEvent() {
	val = Serial.read();
	if (isDigit(val)) {
		liczba += val;
	}
	else if (val == '\n') {
		saveData(liczba);
		liczba = "";
	}
	else if (val == 'P') {
		_receivingPath = true;
	}
	else if (val == 'G') {
		_receivingGrip = true;
	}
	else if (val == 'T') {
		sendPath();
	}
}

void saveData(String val) {
	if (_receivingPath) {
		if (!_lerpPointsAmtReceived) {
			digitalWrite(12, HIGH);
			pathAmt = val.toInt();
			_lerpPointsAmtReceived = true;
		}
		else {
			path[cntr0][cntr1] = val.toInt();
			cntr1++;
			if (cntr1 == MAX_SERVOS) {
				cntr1 = 0;
				cntr0++;
				if (cntr0 == pathAmt) {
					cntr0 = 0;
					_receivingPath = false;
					_lerpPointsAmtReceived = false;
					movement();
					Serial.print("F");
					for (int i = 0; i < pathAmt; i++) {
						for (int j = 0; j < MAX_SERVOS + 1; j++) {
							path[i][j] = 0;
						}
					}
				}
			}
		}
	}
	else if (_receivingGrip) {
		grip = val.toInt();
		_receivingGrip = false;
		setGripper();
		Serial.print("F");
	}

}

void movement() {
	roznica();
	/*for (int i = 0; i < pathAmt; i++) {
		for (int j = 0; j < MAX_SERVOS + 1; j++) {
			Serial.print(path[i][j]);
			Serial.print(" ");
		}
		Serial.println("");
	}*/
	digitalWrite(12, LOW);
	for (int s = 0; s < pathAmt - 1; s++) {
		for (int u = 0; u < path[s + 1][MAX_SERVOS]; u++) {
			for (int i = 0; i < 5; i++) {
				driver.setPWM(i, 0, (int)(path[s][i] + (((float)(path[s + 1][i] - path[s][i]) / path[s + 1][MAX_SERVOS]) * (float)u)));
			}	
			delay(12);
		}
	}

}


void setGripper() {
	if (grip >= lastGrip) {
		for (int p = lastGrip; p <= grip; p++) {
			driver.setPWM(SRV5, 0, p);
			delay(20);
		}
	}
	else {
		for (int p = lastGrip; p >= grip; p--) {
			driver.setPWM(SRV5, 0, p);
			delay(20);
		}
	}
	lastGrip = grip;
}

void roznica() {
	int t = 0;
	int diff = 0;

	for (int i = 1; i < pathAmt; i++) {
		for (int j = 0; j < MAX_SERVOS; j++) {
			diff = abs(path[i][j] - path[i - 1][j]);
			if (diff > t) t = diff;
		}
		path[i][MAX_SERVOS] = t;
		t = 0;
	}
}

//hehe


void start() {
	driver.setPWM(SRV0, 0, 350);
	driver.setPWM(SRV1, 0, 382);
	driver.setPWM(SRV2, 0, 347);
	driver.setPWM(SRV3, 0, 355);
	driver.setPWM(SRV4, 0, 364);
	driver.setPWM(SRV5, 0, 362);
}


void establishContact() {
	while (Serial.available() <= 0) {
		Serial.println("A");
		delay(300);
	}
}

void sendPath() {
	for (int i = 0; i < pathAmt; i++) {
		for (int j = 0; j < MAX_SERVOS + 1; j++) {
			Serial.print(path[i][j]);
			Serial.print(" ");
		}
		Serial.println("\r");
	}
}