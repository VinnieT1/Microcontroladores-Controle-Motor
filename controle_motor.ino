#include <LiquidCrystal.h>

typedef enum State {
  FUNCTION_SELECTION,
  RUNNING,
} State;

State state = FUNCTION_SELECTION;

LiquidCrystal lcd(3, 2, 4, 5, 6, 7);
int last_display_update;

const int button_pin = 8;
const int pwm_pin = 10;

// Time and PWM variables
float output_value = 0;
unsigned long start_time = 0;
unsigned long elapsed_time = 0;

// Function variables
float (*functions[])(float) = { exponential_damp, linear, quadratic };
char* function_expressions[] = {"y(t)=1-e^(-t/5)", "y(t) = t/10", "y(t) = (t/2)^2"};
unsigned int function_index = 0;

// Button variables
int last_button_state = HIGH; // Variable to store the previous state of the button
unsigned long last_debounce_time = 0;  // Last time the button state was changed
unsigned long debounce_delay = 50;    // Debounce delay time in milliseconds
bool long_press_detected = false;  // Flag to track long press
bool short_press_detected = false; // Flag to track short press

void setup()
{
  pinMode(10, OUTPUT); // PWM
  pinMode(8, INPUT_PULLUP); // Button

  Serial.begin(9600);
  lcd.begin(16, 2);
  
  handle_display_function();
}
void loop()
{
  switch (state) {
    case FUNCTION_SELECTION:
      detectButtonPress();

      if (long_press_detected) {
        state = RUNNING;
        last_display_update = millis();
        start_time = millis();
      }
      else if (short_press_detected) {
        function_index = (function_index + 1) % 3;
        handle_display_function();
      }
      break;
    case RUNNING:
      elapsed_time = millis() - start_time;
      output_value = functions[function_index](elapsed_time/1000);
      
      analogWrite(pwm_pin, 255*output_value);
      
      if (millis() - last_display_update > 100) {
        handle_display_percentage(output_value);
        last_display_update = millis();
      }
      break;
  }
}

void handle_display_function() {
  lcd.clear();
  lcd.print("Function: ");
  lcd.setCursor(0, 1);
  lcd.print(function_expressions[function_index]);
  lcd.setCursor(0, 0);
}

void handle_display_percentage(float percentage) {
	lcd.clear();
  lcd.print("Motor: ");
  lcd.print((int)(100*percentage));
  lcd.print("%");
}

// Function to detect button press (short and long press)
void detectButtonPress() {
  // Read the state of the button
  int reading = digitalRead(button_pin);
  unsigned long button_press_start_time = 0; // Time when button press started
  
  // Check if the button state has changed
  if (reading != last_button_state) {
    last_debounce_time = millis();  // Reset debounce timer
    long_press_detected = false;    // Reset long press flag when state changes
    short_press_detected = false;   // Reset short press flag when state changes
  }

  // If enough time has passed since the last state change, update the state
  if ((millis() - last_debounce_time) > debounce_delay) {
    // If the button state has changed to LOW (pressed), start timing the press
    if (reading == LOW && last_button_state == HIGH) {
      button_press_start_time = millis();  // Start counting time for the press
    }

    // If the button is held down, check for a long press (e.g., 2 seconds)
    if (reading == LOW && (millis() - button_press_start_time) >= 2000 && !long_press_detected) {
      // Long press detected (2 seconds or more)
      long_press_detected = true;  // Set flag to prevent repeated long press detection
    }

    // If the button was released, detect short press
    if (reading == HIGH && last_button_state == LOW) {
      if (!long_press_detected) {
        // Short press detected (button was pressed and released quickly)
        short_press_detected = true;
      }
    }
  }

  // Save the current reading as the last button state
  last_button_state = reading;
}

float exponential_damp(float t) {
  return (1 - exp(-t/5));
}

float linear(float t) {
	return min(t/10 ,1);
}

float quadratic(float t) {
  return min(t*t/4, 1);
}