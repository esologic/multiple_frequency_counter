/*
 * 3/29/2018 - Devon Bray - http://www.esologic.com/multiple-frequency-counter-arduino/
 */


int pin_100Hz = 2;
int pin_200Hz = 3;

unsigned long previous_time_100Hz;
unsigned long previous_time_200Hz;

void setup() {
  pinMode(pin_100Hz, OUTPUT);
  pinMode(pin_200Hz, OUTPUT);
}

void loop() {

  unsigned long current_time = millis();

  if ( (current_time - previous_time_200Hz) >= 5) {
    digitalWrite(pin_200Hz, HIGH);
    delayMicroseconds(1000);
    digitalWrite(pin_200Hz, LOW);
    previous_time_200Hz = current_time;
  }

  if ( (current_time - previous_time_100Hz) >= 10) {
    digitalWrite(pin_100Hz, HIGH);
    delayMicroseconds(1000);
    digitalWrite(pin_100Hz, LOW);
    previous_time_100Hz = current_time;
  }
  
}

