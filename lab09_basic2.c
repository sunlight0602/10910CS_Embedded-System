#include <Arduino_FreeRTOS.h>

const int photoPin0 = A0;
const int photoPin1 = A1;

const int disPins[8] = {2, 3, 4, 5, 6, 7, 13, 12}; //pins to 7-seg
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

int speed = 0;

void disNum(int num){
  for(int i=0; i<8; i++){
    digitalWrite(disPins[i], data[num][i] == true? HIGH : LOW);  
  }
}

void disAllNum(){
  for(int i=0; i<10; i++){
    disNum(i);
    delay(300);
  }
}

//void vTask0(void *pvParameters) { // define speed
//  for( ;; ){ 
//    if(analogRead(photoPin0) - 75 > analogRead(photoPin1)){
//      if(speed!=9){
//        speed++;
//      }
//    }
//    else if(analogRead(photoPin0) < analogRead(photoPin1) - 75){
//      if(speed!=0){
//        speed--;
//      }
//    }
//
//    delay(300);
//  }
//}

void vTask1(void *pvParameters) { // speed++
  for( ;; ){
    if(analogRead(photoPin0) - 75 > analogRead(photoPin1)){
      if(speed!=9){
        speed++;
      }
    }
    disNum(speed);
    delay(300);
  }
}

void vTask2(void *pvParameters) { // speed--
  for( ;; ){ 
    if(analogRead(photoPin0) < analogRead(photoPin1) - 75){
      if(speed!=0){
        speed--;
      }
    }
    disNum(speed);
    delay(300);
  }
}

void setup() {
  pinMode(photoPin0, INPUT);
  pinMode(photoPin1, INPUT);
  Serial.begin(9600);
  Serial.print("test");
    
  xTaskCreate(vTask1, "vTask1", 128, NULL, 1, NULL);
  xTaskCreate(vTask2, "vTask2", 128, NULL, 1, NULL);

}

void loop() {

}
