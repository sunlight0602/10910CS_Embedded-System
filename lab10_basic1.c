#include <Arduino_FreeRTOS.h> // for TaskHandle_t
#include <Wire.h>  
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
int delay_LED = 10;
//int delay_cacti = 30; // speed
int speed = 8; // 2~15
int is_stop = 0;

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

    Serial.print(analogRead(photoPin1));
    Serial.print(" | ");
    Serial.print(analogRead(photoPin0));
    Serial.print(" | speed: ");
    Serial.print(speed);
    //Serial.print(" | counter: ");
    //Serial.print(counter);
    Serial.print("\n");
    
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
    lcd.setCursor(cactus0_x, 0); // (col, row)
    lcd.write(1); // write char 1 (cactus)
    lcd.setCursor(cactus1_x, 1);
    lcd.write(1);
    
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

void setup(){
  pinMode(photoPin0, INPUT);
  pinMode(photoPin1, INPUT);
  
  Serial.begin(9600); // select 9600 baud
  Serial.print("Start");
  
  lcd.init(); // initialize LCD
  lcd.backlight(); // open LCD backlight

  lcd.createChar(0, dinosaur);
  lcd.createChar(1, cactus);

  //cactus0_x = random(8, 15); // random initial x
  //cactus1_x = random(8, 15);

  xTaskCreate(LCDTask, "LCDTask", 128, NULL, 1, &LCDTaskHandle);
  xTaskCreate(cactiTask, "cactiTask", 128, NULL, 1, NULL);
  xTaskCreate(leftTask, "leftTask", 128, NULL, 1, NULL);
  xTaskCreate(rightTask, "rightTask", 128, NULL, 1, NULL);
}

void loop(){

}
