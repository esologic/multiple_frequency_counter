int TACK_SIG_PIN = 2;
int SPED_SIG_PIN = 3;

#define NUMDIFFS 100

#define NUMEDGESIGS 2

#define TACKSIGINDEX 0
#define SPEDSIGINDEX 1

volatile int period_buffer_indices[NUMEDGESIGS] = { 0 }; 
volatile unsigned long period_buffers[NUMEDGESIGS][NUMDIFFS] = { 0 };
volatile unsigned long previous_edge_times_us[NUMEDGESIGS] = { 0 };
volatile float period_averages_ms[NUMEDGESIGS] = { 0 };
volatile float frequency_averages_hz[NUMEDGESIGS] = { 0 };
volatile bool period_buffer_locked[NUMEDGESIGS] = { false };

void setup() {
  
  Serial.begin(9600);

  pinMode(TACK_SIG_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TACK_SIG_PIN), new_TACK_edge, RISING);

  pinMode(SPED_SIG_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SPED_SIG_PIN), new_SPED_edge, RISING);
}

void loop() {

  calc_freqs();
  
  Serial.print((abs(period_averages_ms[TACKSIGINDEX] - 16)/16)*100);
  Serial.print(",");
  Serial.print((abs(period_averages_ms[SPEDSIGINDEX] - 12)/12)*100);
  Serial.println("");

  
}

void calc_freqs() {
  
  for (int p_index = 0; p_index < NUMEDGESIGS; p_index++) {
  
    float buffer_sum = 0;

    while (period_buffer_locked[p_index]) {};

    period_buffer_locked[p_index] = true;
  
    for (int j = 0; j < NUMDIFFS; j++) {
      buffer_sum += period_buffers[p_index][j];
    }
    
    period_buffer_locked[p_index] = false;

    if (buffer_sum > 0){
      period_averages_ms[p_index] = ((buffer_sum / (float)NUMDIFFS)) / 1000;
      
      frequency_averages_hz[p_index] = 1 / period_averages_ms[p_index] ;
      
    } else {
      period_averages_ms[p_index] = 0;
      frequency_averages_hz[p_index] = 0;
    } 
  }
}


void new_edge(int period_index) {

  unsigned long current = micros();

  if (period_buffer_locked[period_index] == false) {

    period_buffer_locked[period_index] = true;
    
    period_buffers[period_index][period_buffer_indices[period_index]] = current - previous_edge_times_us[period_index];

    period_buffer_locked[period_index] = false;
    
    period_buffer_indices[period_index]++;
    if (period_buffer_indices[period_index] >= NUMDIFFS) {
      period_buffer_indices[period_index] = 0; 
    }  
  }
  
  previous_edge_times_us[period_index] = current;
  
}

void new_TACK_edge() {
  new_edge(TACKSIGINDEX);
}

void new_SPED_edge() {
  new_edge(SPEDSIGINDEX);
}

