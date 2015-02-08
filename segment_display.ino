#include "LedControl.h" 
// Arduino Pin 7 to DIN, 6 to Clk, 5 to LOAD, no.of devices is 1 
LedControl lc=LedControl(12, 11, 10, 1); 

boolean in_number;

long prev_read_number;
long prev_read_number_ms = 0;
long last_read_number = 0;
long last_read_number_ms = 0;
long value = 25000;

void setup() 
{
  Serial.begin(9600); 

  // Initialize the MAX7219 device 
  lc.shutdown(0, false); // Enable display
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
  lc.clearDisplay(0);
 
  initIdle();  
  in_number = false;
}
 
void loop()
{
  if (in_number)
  {
    numberStep();
  }
  else 
  {
    idleStep();
  }
  readSerial();
  return;
}

boolean have_value = false;
long read_value;
long last_data_ms = millis();

void readSerial()
{
  while (Serial.available()) {
    last_data_ms = millis();
    byte ch = Serial.read();
    if (ch == 13 || ch == 10) {
      if (have_value)
      {
        Serial.println("read value");
        Serial.println(read_value);
        prev_read_number = last_read_number;
        prev_read_number_ms = last_read_number_ms;
        last_read_number = read_value;
        last_read_number_ms = millis();
        have_value = false;
        if (!in_number) {
          initNumber();
          in_number = true;
        }
        return;
      }
      have_value = false;
    }
    else
    {
      if (ch >= '0' && ch <= '9') 
      {
        int digit = ch - '0';
        if (have_value)
        {
          read_value *= 10;
        }
        else
        {
          read_value = 0;
          have_value = true;
        }
        read_value += digit;
      }
    }
  }
}

void initNumber()
{
  lc.setIntensity(0, 10); // Set brightness level (0 is min, 15 is max)
}

void numberStep()
{
  if (!Serial.available() && millis() - last_data_ms > 60000)
  {
    initIdle();
    in_number = false;
    return;
  }
 
  // should be slowing incrementing value, not just setting here
  value = last_read_number;
  if (prev_read_number_ms > 0 && last_read_number > prev_read_number) {
    // interpolate
    value += (last_read_number - prev_read_number) * (millis() - last_read_number_ms) / (last_read_number_ms - prev_read_number_ms);
  }
  long ten = 1;
  for(int i=0; i<8; i++)
  {
      if (i != 0 && value / ten == 0) {
        lc.setChar(0, i, ' ', false);
      } else
      {
        lc.setDigit(0, i, (value / ten) % 10, false);
        ten *= 10;
      }
  }
  value++;
}

int sequence[][2] = {
  { 0, 1 },
  { 0, 2 },
  { 0, 3 },
  { 0, 4 },
  { 1, 4 },
  { 2, 4 },
  { 3, 4 },
  { 4, 4 },
  { 5, 4 },
  { 6, 4 },
  { 7, 4 },
  { 7, 5 },
  { 7, 6 },
  { 7, 1 },
  { 6, 1 },
  { 5, 1 },
  { 4, 1 },
  { 3, 1 },
  { 2, 1 },
  { 1, 1 },
};
#define LEN_SEQUENCE (sizeof(sequence) / sizeof(sequence[0]))
boolean state;
boolean change_state;
int index;
int reset_index;
int intensity;

void initIdle()
{
  change_state = false;
  state = true;
  index = 0;
  reset_index = 0;  
  intensity = 0;
  
  lc.setIntensity(0,intensity);
  lc.clearDisplay(0);

}

void idleStep() 
{
  int row = sequence[index][0];
  int col = sequence[index][1];
  lc.setLed(0, row, col, state); 
  index++;
  if (index == LEN_SEQUENCE)
  {
    index = 0;
  }
  if (change_state)
  {
    state = !state;
    change_state = false;
  }
  else
  {
    if (!change_state && index == reset_index)
    {
      change_state = true;
      reset_index++;
      if (reset_index == LEN_SEQUENCE)
      {
        reset_index = 0;
        intensity++;
        if (intensity > 15)
        {
          intensity = 0;
        }
        lc.setIntensity(0, intensity);
        Serial.println(intensity);
      }
    }
  }
  /*
  lc.setLed(0, row, col, state); 
  col++;
  if (col == 7) 
  {
    col = 0;
    row++;
    if (row == 8)
    {
      row = 0;
      state = !state;
    }
  }
  */
  delay(100); 
}
