#include "Arduino.h"
#include <Servo.h>
#include <cstdlib>
#include <string.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 1
#define CS_PIN D2

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET    -1  // Reset pin not used with I2C
#define SCREEN_ADDRESS 0x3C  // Common I2C address for SSD1306
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

unsigned long previousMillis = 0;
unsigned long elapsedSeconds = 0;

char timeStr[9];

MD_Parola ledMatrix = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char* message = "1234";
int currentCharIndex = 0;
unsigned long lastUpdateI2C = 0;
unsigned long lastUpdateSerial = 0;
const unsigned long interval = 150;

const int ANGLE_MAX = 180;
const int ANGLE_MIN = 0;

int red_LED_pin = D3;
int green_LED_pin = D0;
int button_pin = D8;
int servo_pin = D4;

Servo servo;
int angle = 0;
int analogValue;

int angle_sequence[3];
int angle_game_index = 0;
int game_started = 0;

void setup()
{
  Wire.begin(D1, D6);  // SDA = D1, SCL = D6 (adjust if needed)
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 15);
  display.print("Loading...");
  display.display();
  delay(1000);
  display.clearDisplay();
  display.display();


  ledMatrix.begin();         // initialize led matrix 
  ledMatrix.setIntensity(10); // set the brightness of the LED matrix display (from 0 to 15)
  ledMatrix.displayClear();  // clear led matrix display

  pinMode(red_LED_pin, OUTPUT);
  pinMode(green_LED_pin, OUTPUT);
  pinMode(button_pin, INPUT);
  servo.attach(servo_pin, (uint16_t) 1000, (uint16_t) 2000, 0);

  Serial.begin(9600);

}

void servo_smooth_rotate(int initial_angle, int final_angle){
  if (initial_angle < final_angle){
    for(int i = initial_angle; i < final_angle; i++)  
    {                                  
      servo.write(i);               
      delay(15);                   
    } 
  } else{
    for(int i = initial_angle; i > final_angle; i--)    
    {                                
      servo.write(i);           
      delay(15);       
    } 
  }
}

void servo_rotate(){
  servo_smooth_rotate(angle, 0);
   for(angle = 0; angle < 360; angle++)  
   {                                  
     servo.write(angle);               
     delay(15);                   
   } 
   delay(1000);
   for(angle = 360; angle > 0; angle--)    
   {                                
     servo.write(angle);           
     delay(15);       
   } 
}

void change_servo_angle_by_potentiometer(){
  int initial_angle = angle;
  analogValue = analogRead(A0);
  int new_angle = map(analogValue, 0, 1023, ANGLE_MIN, ANGLE_MAX);
  servo_smooth_rotate(initial_angle, new_angle);
  angle = new_angle;
}

int random_angle(){
  int random_num = rand() % 7;
  if (random_num == 0){
    return 0;
  }else if (random_num == 1){
    return 30;
  }else if (random_num == 2){
    return 60;
  }else if (random_num == 3){
    return 90;
  }else if (random_num == 4){
    return 120;
  }else if (random_num == 5){
    return 150;
  } else{
    return 180;
  }
}

void blink_LED(int led_pin){
  digitalWrite(led_pin, HIGH);
  delay(800);
  digitalWrite(led_pin, LOW);   
  delay(200);
}

void initiate_game(){
  Serial.print("Initiating new game with values:");
  Serial.print("\n");
  blink_LED(red_LED_pin);
  blink_LED(green_LED_pin);
  angle_game_index = 0;
  for (int i = 0; i < 3; i++){
    angle_sequence[i] = random_angle();
    servo.write(angle_sequence[i]);
    Serial.print(angle_sequence[i]);
    Serial.print("\n");
    delay(1000);
  }
  game_started = 1;
  Serial.print("####################");
  Serial.print("\n");
  blink_LED(green_LED_pin);
  blink_LED(green_LED_pin);
  blink_LED(green_LED_pin);
}

int read_potentiometer_value(){
  analogValue = analogRead(A0);
  return map(analogValue, 0, 1023, ANGLE_MIN, ANGLE_MAX);
}

void check_user_pattern(){
  if (abs(read_potentiometer_value() - angle_sequence[angle_game_index]) <= 10){
    angle_game_index++;
    blink_LED(green_LED_pin);
  } else{
    blink_LED(red_LED_pin);
    Serial.print(read_potentiometer_value());
    Serial.print("\n");
  }

  if (angle_game_index >= 3){
    digitalWrite(green_LED_pin, HIGH);
    game_started = 0;
  }
}

void loop()
{
  Serial.print("####\nStart of new loop\n####\n");
  bool button_value = digitalRead(button_pin);

  unsigned long currentMillis = millis();

  // Count seconds manually
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    elapsedSeconds++;
  }

  // Convert to HH:MM:SS
  int hours = (elapsedSeconds / 3600) % 24;
  int minutes = (elapsedSeconds / 60) % 60;
  int seconds = elapsedSeconds % 60;

  (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  if ((currentMillis - lastUpdateI2C >= 1000)) {
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
    Serial.print("Updating I2C display");
    lastUpdateI2C = currentMillis;
    // Display time
    display.clearDisplay();
    display.setCursor(10, 15);
    display.print(timeStr);
    display.display();
  }


  if (currentMillis - lastUpdateSerial >= interval) {
    lastUpdateSerial = currentMillis;

    // Show next character
    char currentChar[2] = { message[currentCharIndex], '\0' };
    ledMatrix.displayClear();
    ledMatrix.setTextAlignment(PA_CENTER);
    ledMatrix.print(currentChar);

    // Advance index
    currentCharIndex++;
    if (currentCharIndex >= (int) strlen(message)) {
      currentCharIndex = 0;
    }
  }
    
  // if (ledMatrix.displayAnimate()) {
  //   ledMatrix.displayText("H", PA_CENTER, 100, 0, PA_PRINT, PA_NO_EFFECT);
  // }

  if (button_value){
    delay(1000);
    button_value = digitalRead(button_pin);
    if (button_value){
      // long press
      initiate_game();
    }
    else if (game_started){
      // short press
      check_user_pattern();
    }

  } 
  else{
    servo.write(read_potentiometer_value());
  }
  delay(1000);
  Serial.print("$$$$\nEnd of the loop\n$$$$\n");
}