#include "LedControl.h" 
// Arduino Pin 7 to DIN, 6 to Clk, 5 to LOAD, no.of devices is 1 
LedControl lc=LedControl(12, 11, 10, 1); 

boolean in_number;

long prev_read_number = 0;
long prev_read_number_ms = 0;
long last_read_number = 0;
long last_read_number_ms = 0;
long rate = 0;
long value = 0;
long last_step_ms = 0;

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
        if (value == 0) {
          value = last_read_number;
          Serial.println("setting value");
          Serial.println(value);
          last_step_ms = millis();
        }
        if (last_read_number > 0 && prev_read_number > 0) {
          long instantaneous_rate = 1000 * (last_read_number - prev_read_number) / (last_read_number_ms - prev_read_number_ms);
          // weighted average to change speeds slowly
          rate = (3*rate + instantaneous_rate / 4);
        }
        if (last_read_number < value) {
          Serial.println("lowering rate");
          rate = rate / 2;
        }
        if (rate < 1) {
          rate = 1;
        }
        if (last_read_number < value * 9 / 10) {
          value = last_read_number;
          last_step_ms = millis();
          Serial.println("setting value due to inaccuracy");
          Serial.println(value);
        }
        Serial.println("rate is now");
        Serial.println(rate);
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
 
  // interpolate
  long diff = rate * (millis() - last_step_ms) / 1000;
  if (diff > 0) {
    value += diff;
    //Serial.println("diff put us to");
    //Serial.println(value);
    last_step_ms = millis();
  }

  long ten = 1;
  for (int i=0; i<8; i++)
  {
      if (i != 0 && value / ten == 0) {
        lc.setChar(0, i, ' ', false);
      } else
      {
        lc.setDigit(0, i, (value / ten) % 10, false);
        ten *= 10;
      }
  }
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
