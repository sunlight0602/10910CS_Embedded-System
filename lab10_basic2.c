#include <Arduino_FreeRTOS.h> // for TaskHandle_t
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F,16,2); // 0x27 or 0x3F

// LED chars
byte dinosaur[8]  = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};

// Task Handler
TaskHandle_t LCDTaskHandle;

// Photon
const int photoPin0 = A0; // right
const int photoPin1 = A1; // left

int cactus0_x = 8; // initial x of cactus
int cactus1_x = 14;
int cactus0_y = 0; // const
int cactus1_y = 1; // const
int delay_LED = 10;
//int delay_cacti = 30; // speed
int speed = 8; // 2~15
int is_stop = 0;

int dino_x = 0;
int dino_y = 0;
int score = 0;

// Keymap
char keymap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte colPins[4] = {9, 8, 7, 6};
byte rowPins[4] = {13, 12, 11, 10};

void cactiTask(void *pvParameters){
  int counter = 0;
  for(;;){
    
    if(is_stop==1){
      // do nothing
    }
    else if(counter % speed){
      // smaller speed, 0 will appear more frequently
      cactus0_x = cactus0_x - 1;
      cactus1_x = cactus1_x - 1;
    }

    counter = (counter+1) % 10000; // avoid overflow

//    Serial.print(analogRead(photoPin1));
//    Serial.print(" | ");
//    Serial.print(analogRead(photoPin0));
//    Serial.print(F(" | speed: "));
//    Serial.print(speed);
//    //Serial.print(" | counter: ");
//    //Serial.print(counter);
//    Serial.print(" | cactus0_x: ");
//    Serial.print(cactus0_x);
//    Serial.print(" | cactus0_y: ");
//    Serial.print(cactus0_y);
//    Serial.print(" | dino_x: ");
//    Serial.print(dino_x);
//    Serial.print(" | dino_y: ");
//    Serial.print(dino_y);
//    Serial.print("\n");

    if((dino_x==cactus0_x && dino_y==cactus0_y) || (dino_x==cactus1_x && dino_y==cactus1_y)){
      vTaskSuspend(LCDTaskHandle);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("Game Over"));
      lcd.setCursor(2,1);
      lcd.print(F("Score"));
      lcd.setCursor(8,1);
      lcd.print(score);
      Serial.print(F("Restart Game\n"));
      delay(2000);
      score = 0;
      vTaskResume(LCDTaskHandle);
    }
    else if(dino_x==cactus0_x || dino_x==cactus1_x){
      score += 1;
    }
    
    vTaskDelay(delay_LED); // Don't move too fast
  }
}

void LCDTask(void *pvParameters){
  for(;;){
    lcd.clear();

    if(cactus0_x <= 0){
      cactus0_x = random(8, 15); // random initial x
      if(cactus0_x==cactus1_x){ // avoid same x
        cactus0_x = cactus0_x-2;
      }
    }
    if(cactus1_x <= 0){
      cactus1_x = random(8, 15);
      if(cactus1_x==cactus0_x){
        cactus1_x = cactus1_x-2;
      }
    }
    lcd.setCursor(cactus0_x, cactus0_y); // (col, row)
    lcd.write(1); // write char 1 (cactus)
    lcd.setCursor(cactus1_x, cactus1_y);
    lcd.write(1);
    lcd.setCursor(dino_x, dino_y);
    lcd.write(0);

//    if(dino_x==cactus0_x && dino_y==cactus0_y){
//      lcd.clear();
//      lcd.setCursor(0,0);
//      lcd.print("Game Over");
//      delay(1000);
//      Serial.print("Restart Game\n");
//    }
    
    vTaskDelay(delay_LED); // stable the LEDs
  }
}

void leftTask(void *pvParameters){
  for(;;){
    int photo_right = analogRead(photoPin0);
    int photo_left = analogRead(photoPin1);

    if(photo_left<=700 && photo_right<=700){
      vTaskSuspend(LCDTaskHandle);
      is_stop = 1;
    }
    else{
      vTaskResume(LCDTaskHandle);
      is_stop = 0;
    }
    
    if(photo_left < photo_right-75 && is_stop==0){ //Left photoresistor dark for deceleration
      if(speed!=2){
        speed--;
      }
    }
    
    delay(300); // vTaskDelay?
  }
}

void rightTask(void *pvParameters){
  for(;;){
    int photo_right = analogRead(photoPin0);
    int photo_left = analogRead(photoPin1);

    if(photo_left<=700 && photo_right<=700){
      vTaskSuspend(LCDTaskHandle);
      is_stop = 1;
    }
    else{
      vTaskResume(LCDTaskHandle);
      is_stop = 0;
    }
    
    if(photo_right < photo_left-75 && is_stop==0){ //Right photoresistor dark for acceleration
      if(speed!=15){
        speed++;
      }
    }
    
    delay(300); // vTaskDelay?
  }
}

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);

void keypadTask(void *pvParameters){
  
  for(;;){
    
    char key = myKeypad.getKey();
    if(key=='2'){
      if(dino_y==1){
        dino_y = 0;
      }
      //Serial.print(key);
    }
    else if(key=='8'){
      if(dino_y==0){
        dino_y = 1;
      }
      //Serial.print(key);
    }

    vTaskDelay(delay_LED);
  }
}

void setup(){
  pinMode(photoPin0, INPUT);
  pinMode(photoPin1, INPUT);
  
  Serial.begin(9600); // select 9600 baud
  Serial.print(F("Start\n"));
  
  lcd.init(); // initialize LCD
  lcd.backlight(); // open LCD backlight

  lcd.createChar(0, dinosaur);
  lcd.createChar(1, cactus);

  //cactus0_x = random(8, 15); // random initial x
  //cactus1_x = random(8, 15);

  xTaskCreate(LCDTask, "LCDTask", 100, NULL, 1, &LCDTaskHandle);
  xTaskCreate(cactiTask, "cactiTask", 100, NULL, 1, NULL);
  xTaskCreate(leftTask, "leftTask", 64, NULL, 1, NULL);
  xTaskCreate(rightTask, "rightTask", 64, NULL, 1, NULL);
  //Use memory carefully and set the size of task stack properly ??
  xTaskCreate(keypadTask, "keypadTask", 80, NULL, 1, NULL);
}

void loop(){

}
