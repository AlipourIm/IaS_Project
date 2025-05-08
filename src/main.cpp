#include "Arduino.h"
#include <Servo.h>
#include <cstdlib>
#include <string.h>

const int ANGLE_MAX = 180;
const int ANGLE_MIN = 0;

int red_LED_pin = D7;
int green_LED_pin = D6;
int button_pin = D8;
Servo servo;
int angle = 0;
int analogValue;

int angle_sequence[3];
int angle_game_index = 0;

void setup()
{
  pinMode(red_LED_pin, OUTPUT);
  pinMode(green_LED_pin, OUTPUT);
  pinMode(button_pin, INPUT);
  servo.attach(D4, (uint16_t) 1000, (uint16_t) 2000, 0);
}

void send_SOS(){
  for (int i = 0; i < 3; i++){
    digitalWrite(red_LED_pin, HIGH);
    delay(200);
    digitalWrite(red_LED_pin, LOW); 
    delay(200); 
  }

  digitalWrite(red_LED_pin, HIGH);
  digitalWrite(green_LED_pin, HIGH);
  delay(800);
  digitalWrite(red_LED_pin, LOW);
  digitalWrite(green_LED_pin, LOW); 
  delay(200);  

  for (int i = 0; i < 3; i++){
    digitalWrite(red_LED_pin, HIGH);
    delay(200);
    digitalWrite(red_LED_pin, LOW); 
    delay(200);
  }
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
}

void initiate_game(){
  blink_LED(red_LED_pin);
  blink_LED(green_LED_pin);
  angle_game_index = 0;
  for (int i = 0; i < 3; i++){
    angle_sequence[i] = random_angle();
    servo.write(angle_sequence[i]);
    delay(1000);
  }
  blink_LED(green_LED_pin);
}

int read_potentiometer_value(){
  analogValue = analogRead(A0);
  return map(analogValue, 0, 1023, ANGLE_MIN, ANGLE_MAX);
}

void check_user_pattern(){
  if (abs(read_potentiometer_value() - angle_sequence[angle_game_index]) <= 5){
    angle_game_index++;
    blink_LED(green_LED_pin);
  } else{
    blink_LED(red_LED_pin);
  }

  if (angle_game_index == 3){
    digitalWrite(green_LED_pin, HIGH);
  }
}

void loop()
{
  bool button_value = digitalRead(button_pin);
  if (button_value){
    //send_SOS();
    //servo_rotate();
    delay(1000);
    button_value = digitalRead(button_pin);
    if (button_value){
      // long press
      initiate_game();
    }
    else{
      // short press
      check_user_pattern();
    }

  } 
  else{
    servo.write(read_potentiometer_value());
  }
  delay(150);
}