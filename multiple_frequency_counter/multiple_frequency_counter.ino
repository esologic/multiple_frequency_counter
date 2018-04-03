/*
 * 3/29/2018 - Devon Bray - http://www.esologic.com/multiple-frequency-counter-arduino/
 * 
 * I've written most of the important notes as comments in the source, but a couple more details:
 * 
 * - The important data is stored in `period_averages_ms` and `frequency_averages_hz`. You address them using the indices defined at the top of the file. These arrays get updated each time `compute_counts()` is called. Keep it `compute_counts()` somewhere in the main() loop. 
 * 
 * - You could easily add more frequencies, you just have to `NUMSIGS`, make a specific ISR, and another `attachInterrupt` line in setup()
 * 
 * - It uses [interrupts](https://playground.arduino.cc/Code/Interrupts) which might not be right for your proejct, but normally shouldn't get in the way of too much stuff.
 * 
 * - If the ISR hasn't seen a new edge in 1000000us, both `period_averages_ms[p_index]` and `frequency_averages_hz[p_index]` will be set to zero!
 * - This means that slowest frequency that this code can detect is 1hz!
 * 
 */

int freq_pin_1 = 2; // the pin connected to the first signal, must be an interrupt pin! See the arduino docs
int freq_pin_2 = 3; // the pin connected to the second signal, must be an interrupt pin! See the arduino docs

#define BUFFSIZE 100 // a rolling average of the frequency/period is computed, and this is the size of that buffer

#define NUMSIGS 2
#define FREQ1INDEX 0
#define FREQ2INDEX 1

volatile int period_buffer_indices[NUMSIGS] = { 0 }; // the location of the index for adding to the rolling buffer average
volatile unsigned long period_buffers[NUMSIGS][BUFFSIZE] = { 0 }; // the buffers
volatile unsigned long previous_edge_times_us[NUMSIGS] = { 0 }; // the time that the previous edge came in in microseconds
volatile float period_averages_ms[NUMSIGS] = { 0 }; // the period time of a given signal in milliseconds
volatile float frequency_averages_hz[NUMSIGS] = { 0 }; // the frequency of a given signal in hertz
volatile bool period_buffer_locked[NUMSIGS] = { false }; // spin locks for the different buffers

void setup() {
  
  Serial.begin(9600);

  // the pins must be mapped to their ISRs 

  pinMode(freq_pin_1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(freq_pin_1), new_freq1_edge, RISING); // you could change this mode to whatever you were looking for, FALLING, CHANGE etc.

  pinMode(freq_pin_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(freq_pin_2), new_freq2_edge, RISING);
}

void loop() {

  compute_counts();
  
  Serial.print("Pin 1: ");
  Serial.print(period_averages_ms[FREQ1INDEX]);
  Serial.print("ms, ");
  Serial.print(frequency_averages_hz[FREQ1INDEX]);
  Serial.print(" hz");
  
  Serial.print(" - Pin 2: ");
  Serial.print(period_averages_ms[FREQ2INDEX]);
  Serial.print("ms, ");
  Serial.print(frequency_averages_hz[FREQ2INDEX]);
  Serial.print(" hz");
  Serial.println("");  
}

void compute_counts() {

  // computes the average of the buffer for a given signal. Must be called before using the period_averages_ms or frequency_averages_hz buffers.
  
  for (int p_index = 0; p_index < NUMSIGS; p_index++) {
  
    float buffer_sum = 0;

    while (period_buffer_locked[p_index]) {}; // wait around for the ISR to finish
    
    period_buffer_locked[p_index] = true; // ISR won't add new data to `period_buffers`
    if ((micros() - previous_edge_times_us[p_index]) < 1000000) {
      for (int j = 0; j < BUFFSIZE; j++) {
        buffer_sum += period_buffers[p_index][j];
      }
    }
    period_buffer_locked[p_index] = false; // ISR will now add new data to `period_buffers`
    
    if (buffer_sum > 0){
      period_averages_ms[p_index] = ((buffer_sum / (float)BUFFSIZE)) / 1000;
      frequency_averages_hz[p_index] = (1 / period_averages_ms[p_index]) * 1000;  
    } 
    else {
      period_averages_ms[p_index] = 0;
      frequency_averages_hz[p_index] = 0;
    }
        
  }
}

void new_edge(int period_index) {

  unsigned long current = micros();

  if (period_buffer_locked[period_index] == false) { // if compute_counts is using the buffer, skip adding to it because that process isn't atomic

    period_buffer_locked[period_index] = true;
    
    period_buffers[period_index][period_buffer_indices[period_index]] = current - previous_edge_times_us[period_index];

    period_buffer_locked[period_index] = false;
    
    period_buffer_indices[period_index]++;
    if (period_buffer_indices[period_index] >= BUFFSIZE) {
      period_buffer_indices[period_index] = 0; 
    }  
  }
  
  previous_edge_times_us[period_index] = current; // but make sure the new time is set because this operation is atomic
  
}

void new_freq1_edge() {
  new_edge(FREQ1INDEX);
}

void new_freq2_edge() {
  new_edge(FREQ2INDEX);
}

