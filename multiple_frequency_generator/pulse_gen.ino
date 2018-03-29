#define time_between_pulses_ms 16

int Hz30_pin = 2;
int Hz60_pin = 3;
int Hz80_pin = 4;

bool Hz30_pin_state = false;
bool Hz60_pin_state = false;
bool Hz80_pin_state = false;

unsigned long previous_time_slow;
unsigned long previous_time_fast;

void setup() {
  pinMode(Hz30_pin, OUTPUT);
  pinMode(Hz60_pin, OUTPUT);
  pinMode(Hz80_pin, OUTPUT);
}

void loop() {

  unsigned long current_time = millis();

  if ( (current_time - previous_time_slow) >= time_between_pulses_ms) {
    digitalWrite(Hz30_pin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(Hz30_pin, LOW);
    previous_time_slow = current_time;
  }

  if ( (current_time - previous_time_fast) >= 12) {
    digitalWrite(Hz60_pin, HIGH);
    delayMicroseconds(1000);
    digitalWrite(Hz60_pin, LOW);
    previous_time_fast = current_time;
  }
  
}

