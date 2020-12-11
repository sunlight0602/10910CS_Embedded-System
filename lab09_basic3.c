#include <Arduino_FreeRTOS.h>

const int disPins[8] = {2, 3, 4, 5, 6, 7, 13, 12}; //pins to 7-seg.
const boolean data[10][8] = {
  {true, true, true, true, true, true, false, true}, // 0
  {false, true, true, false, false, false, false, true}, // 1
  {true, true, false, true, true, false, true, true}, // 2
  {true, true, true, true, false, false, true, true}, // 3
  {false, true, true, false, false, true, true, true}, // 4
  {true, false, true, true, false, true, true, true}, // 5
  {true, false, true, true, true, true, true, true}, // 6
  {true, true, true, false, false, false, false, true}, // 7
  {true, true, true, true, true, true, true, true}, // 8
  {true, true, true, true, false, true, true, true}, // 9
};

//LEDs
const int ledPinG = 9;
const int ledPinR = 10;

// Photon detect
const int photoPin0 = A0;
const int photoPin1 = A1;

// Ultrasonic 
const int trigPin = 11;  
const int echoPin = A2;

int speed = 0;
int sonic_distance = 0;
int distance = 0;
int duration = 0;
int ignore_photodet = 0; // 0,1

void disNum(int num){
  for(int i = 0; i < 8; i++){
    digitalWrite(disPins[i], data[num][i] == true? HIGH : LOW);  
  }
}

void disAllNum(){
  for(int i = 0; i < 10; i++){
    disNum(i);
    delay(300);
  }
}

void t_potoDet0(void *pvParameters){
  for( ;; ){
    int photo_left = analogRead(photoPin0);
    int photo_right = analogRead(photoPin1);
    if(ignore_photodet==0){
      if(photo_left-75 > photo_right){ // stabalize
        if(speed!=9){
          speed++;
        }
      }
    }
    delay(300); 
   }
}

void t_potoDet1(void *pvParameters){
  for( ;; ){
    int photo_left = analogRead(photoPin0);
    int photo_right = analogRead(photoPin1);
    if(ignore_photodet==0){
      if(photo_left < photo_right-75){
        if(speed!=0){
          speed--;
        }
      }
    }
    delay(300);
   }
}

void t_disNum(void *pvParameters){
  for( ;; ){
    disNum(speed);
    if(speed < 4){
      digitalWrite(ledPinG, HIGH);
      digitalWrite(ledPinR, LOW);
    }
    else if(speed >= 4 && speed < 7){
      digitalWrite(ledPinG, HIGH);
      digitalWrite(ledPinR, HIGH);
    }
    else if(speed >= 7){
      digitalWrite(ledPinG, LOW);
      digitalWrite(ledPinR, HIGH);
    }
  }
}

void t_sonic(void *pvParameters){
  for( ;; ){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH); // Give trig HIGH for 10 ms
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    pinMode(echoPin, INPUT); // read echo
    duration = pulseIn(echoPin, HIGH); // time when recieve HIGH
   
    distance = (duration/2) / 29.1; // turn time to cm
    if(distance > 15){
      ignore_photodet = 1;
      speed = 0;
    }
    else{
      ignore_photodet = 0;
    }
    
    Serial.print(distance);
    Serial.print(" | ");
    Serial.print(ignore_photodet);
    Serial.print("\n");

    delay(300);
  }
}

void setup(){
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinR, OUTPUT);
  
  pinMode(photoPin0, INPUT);
  pinMode(photoPin1, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600);
  Serial.print("Start");
    
  xTaskCreate(t_potoDet0, "t_potoDet0", 128, NULL, 1, NULL);
  xTaskCreate(t_potoDet1, "t_potoDet1", 128, NULL, 1, NULL);
  xTaskCreate(t_disNum, "t_disNum", 128, NULL, 1, NULL);
  xTaskCreate(t_sonic, "t_sonic", 128, NULL, 1, NULL);

  digitalWrite(ledPinG, LOW);
  digitalWrite(ledPinR, LOW);
}

void loop(){
 
}
