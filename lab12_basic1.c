#include <Arduino_FreeRTOS.h> // for TaskHandle_t
#include <Wire.h>
//#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Queue.h>
#include <semphr.h>

LiquidCrystal_I2C lcd(0x3F,16,2); // 0x27 or 0x3F

// LED chars
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

// Task Handler
TaskHandle_t LCDTaskHandle;
SemaphoreHandle_t gatekeeper = 0;
SemaphoreHandle_t binary_sem;

const int delay_LED = 10;

typedef struct{
  int x = random(1,15);
  int y = random(0,1);
} coor;

int egg_idx = 0;
int egg_x[3] = {-1,-1,-1};
int egg_y[3] = {-1,-1,-1};
int egg_type[3] = {0};

int buf[2][16];

// Keymap
char keymap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte colPins[4] = {9, 8, 7, 6};
byte rowPins[4] = {13, 12, 11, 10};

// Joystick
const int xAxis = A0;
const int yAxis = A1;
const int buttonJoystick = A2;

void restartGame(){
  // Reset eggs
  egg_idx = 0;
  for(int i=0; i<3; i++){
    egg_x[i] = -1;
    egg_y[i] = -1;
    egg_type[i] = 0;
  }
  
  return;
}

coor dino_c;
coor cac1_c;

void displayTask(void *pvParameters){
  
  int game_continue = 1; // 0,1
  
  for(;;){
    if(xSemaphoreTake(gatekeeper, 100)){
      Serial.println("displayTask got access");
      
      lcd.clear();
      for(int i=0; i<2; i++){
        for(int j=0; j<16; j++){
          if(buf[i][j]==1){
            lcd.setCursor(j, i);
            lcd.write(0); // dino
          }
          else if(buf[i][j]==2){
            lcd.setCursor(j, i);
            lcd.write(1); // cactus
          }
          else if(buf[i][j]==4){
            lcd.setCursor(j, i);
            lcd.write(2); // egg
          }
          else if(buf[i][j]==5){
            lcd.setCursor(j, i);
            lcd.write(3); // broken
            game_continue = 0;
          }
        }
      }
  
      if(game_continue==0){
        delay(1000);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("broken");
        delay(1000);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Game Over");
        delay(1000);
  
        restartGame();
        game_continue = 1;
      }
      else if(egg_idx==3){
        delay(1000);
  
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Succeed!");
        delay(1000);
  
        restartGame();
      }

      xSemaphoreGive(gatekeeper);
    }
    else{
      Serial.println("displayTask cannot access");
    }
    
    vTaskDelay(delay_LED); // stable the LEDs
  }
}

void cactusTask(void *pvParameters){
  for(;;){
    if(xSemaphoreTake(binary_sem, 100)){
      Serial.println("enter cactusTask binary_sem");
      
      while(1){ // random until valid
        int dir = random(0, 4); // Choose between up, down, right, left
      
        // Move cactus
        if(dir==0){ // left
          if(cac1_c.x!=0){
            if(cac1_c.x-1!=dino_c.x || cac1_c.y!=dino_c.y){
              cac1_c.x = cac1_c.x-1;
              break;
            }
          }
        }
        else if(dir==1){ // right
          if(cac1_c.x!=15){
            if(cac1_c.x+1!=dino_c.x || cac1_c.y!=dino_c.y){
              cac1_c.x = cac1_c.x+1;
              break;
            }
          }
        }
        else if(dir==2){ // up
          if(cac1_c.y!=0){
            if(cac1_c.x!=dino_c.x || cac1_c.y-1!=dino_c.y){
              cac1_c.y = 0;
              break;
            }
          }
        }
        else if(dir==3){ // down
          if(cac1_c.y!=1){
            if(cac1_c.x!=dino_c.x || cac1_c.y+1!=dino_c.y){
              cac1_c.y = 1;
              break;
            }
          }
        }
      }
    
      // Collide with egg
      for(int i=0; i<egg_idx; i++){
        if(cac1_c.x==egg_x[i] && cac1_c.y==egg_y[i]){
          //Gameover
          egg_type[i] = 0; // Change img
        }
      }
      
      //xSemaphoreGive(binary_sem);
    }
    else{
      Serial.println("cannot cactusTask binary_sem");
    }
    
    vTaskDelay(delay_LED);
  }
}


//Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);
void dinoTask(void *pvParameters){
  
  for(;;){
    
    //char key = myKeypad.getKey();

    // Read input from joystick
    int xVal = analogRead(xAxis);
    int yVal = analogRead(yAxis);
    int isPress = digitalRead(buttonJoystick);
//    Serial.print(xVal);
//    Serial.print(" | ");
//    Serial.print(yVal);
//    Serial.print(" | ");
//    Serial.println(isPress); // joystick problem ?

    // maybe a debounce here

    // Lay egg
    //if(key=='5' || isPress){ //lay egg
    if(isPress){
      if(dino_c.x!=egg_x[egg_idx-1] || dino_c.y!=egg_y[egg_idx-1]){
        egg_x[egg_idx] = dino_c.x;
        egg_y[egg_idx] = dino_c.y;
        egg_type[egg_idx] = 1;
        egg_idx++;
        continue;
      }
    }

    // Move current dino
    int move_flag = 0;
    if(yVal<490){ // up
      if(dino_c.y==1){
        dino_c.y = 0;
        move_flag = 1;
      }
    }
    else if(yVal>520){ // down
      if(dino_c.y==0){
        dino_c.y = 1;
        move_flag = 1;
      }
    }
    else if(xVal<490){ // left
      if(dino_c.x!=0){
        dino_c.x = dino_c.x-1;
        move_flag = 1;
      }
    }
    else if(xVal>520){ // right
      if(dino_c.x!=15){
        dino_c.x = dino_c.x+1;
        move_flag = 1;
      }
    }

    // Move current cactus
    if(move_flag==1){
      //cac1_c = cactus_move(cac1_c);
      xSemaphoreGive(binary_sem);
      move_flag = 0;
    }

    // update buf map
    if(xSemaphoreTake(gatekeeper, 100)){
      Serial.println("controlTask got access");
      for(int i=0; i<2; i++){
        for(int j=0; j<16; j++){
          
          if(dino_c.y==i && dino_c.x==j){
            buf[i][j] = 1; //"D";
          }
          else if(cac1_c.y==i && cac1_c.x==j){
            buf[i][j] = 2; //"C";
          }
          else{
            buf[i][j] = 3; //"e";
          }
          
          for(int k=0; k<egg_idx; k++){
            if(egg_y[k]==i && egg_x[k]==j){
              if(egg_type[k]==1){
                buf[i][j] = 4; //"E";
              }
              else{
                buf[i][j] = 5; //"B";
              }
            }
          }
          
        }
      }
      xSemaphoreGive(gatekeeper);
    }
    else{
      Serial.println("contralTask cannot access");
    }
    
    /*for(int i=0; i<2; i++){
      for(int j=0; j<16; j++){
        Serial.print(buf[i][j]);
        Serial.print(" ");
      }
      Serial.print("\n");
    }*/

    vTaskDelay(delay_LED);
  }
}

void setup(){
  
  Serial.begin(9600); // select 9600 baud
  Serial.print(F("Start\n"));
  
  lcd.init(); // initialize LCD
  lcd.backlight(); // open LCD backlight

  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cactus);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);

  dino_c.x = 7;
  dino_c.y = 1;
  cac1_c.x = 9;
  cac1_c.y = 0;
  
  gatekeeper = xSemaphoreCreateMutex();
  //vSemaphoreCreateBinary(binary_sem);
  binary_sem = xSemaphoreCreateBinary();
  
  xTaskCreate(dinoTask, "dinoTask", 128, NULL, 1, NULL);
  xTaskCreate(cactusTask, "cactusTask", 60, NULL, 1, NULL);
  xTaskCreate(displayTask, "displayTask", 128, NULL, 1, &LCDTaskHandle);

  vTaskStartScheduler();
  
  //Use memory carefully and set the size of task stack properly
  //Try and error
}

void loop(){

}