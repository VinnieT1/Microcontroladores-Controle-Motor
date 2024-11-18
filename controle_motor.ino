/*
  GRUPO: 
    Gabriel Lucas Bento Germano
    Vinicius Teixeira Pereira Ramos
    Victor Alexandre da Rocha Monteiro Miranda

  PERÍODO: 2024.1
*/

#include <LiquidCrystal.h>

typedef enum State {
  IDLE,
  FUNCTION_SELECTION,
  RUNNING,
} State;

State state = FUNCTION_SELECTION;

LiquidCrystal lcd(3, 9, 4, 5, 6, 7);
unsigned long last_display_update;

const int button_pin = 8;
const int interrupt_pin = 2;
const int pwm_pin = 10;

// Time and PWM variables
float output_value = 0;
unsigned long start_time = 0;
unsigned long elapsed_time = 0;

// Function variables
float (*functions[])(float) = { exponential_damp, linear, quadratic };
char* function_expressions[] = {"y(t)=1-e^(-t/5)", "y(t) = t/10", "y(t) = (t/4)^2"};
unsigned int function_index = 0;

// Button variables
const unsigned long long_press_threshold = 1000; // Tempo em milissegundos para pressionamento longo
unsigned long press_start_time;
unsigned long press_duration;

void setup()
{
  pinMode(10, OUTPUT); // PWM
  pinMode(8, INPUT_PULLUP); // Button

  attachInterrupt(digitalPinToInterrupt(interrupt_pin), handle_button_press, FALLING);
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  handle_display_function();
}

void loop()
{
  switch (state) {
    case IDLE:
      break; // Waits button press
    case FUNCTION_SELECTION: // Handles short and long button press
      press_start_time = millis();

      while (digitalRead(button_pin) == LOW) { // Wait for button release
        press_duration = millis() - press_start_time;

        if (press_duration >= long_press_threshold) {
          break;
        }
      }

      if (press_duration < long_press_threshold) { // Short press
        function_index = (function_index + 1) % 3;
        handle_display_function();
        state = IDLE;
      } else { // Long press
        state = RUNNING;
        last_display_update = millis();
        start_time = millis();
        detachInterrupt(digitalPinToInterrupt(button_pin)); // Disables interruption, no longer wanted
      }
      break;
    case RUNNING:
      elapsed_time = millis() - start_time;
      output_value = functions[function_index]((float)elapsed_time/1000); // Output value from selected function
      
      Serial.print("t: ");
      Serial.print(elapsed_time);
      Serial.print(" y(t): ");
      Serial.print(100*output_value);
      Serial.println("%");
      
      analogWrite(pwm_pin, 255*output_value); // PWM output
    
      if (millis() - last_display_update > 50) {
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

void handle_button_press() {
  state = FUNCTION_SELECTION;
}

float exponential_damp(float t) {
  return (1 - exp(-t/5));
}

float linear(float t) {
	return min(t/10 ,1);
}

float quadratic(float t) {
  return min(t*t/16, 1);
}