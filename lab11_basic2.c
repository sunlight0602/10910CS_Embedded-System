#include <Arduino_FreeRTOS.h> // for TaskHandle_t
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Queue.h>

LiquidCrystal_I2C lcd(0x3F,16,2); // 0x27 or 0x3F

// LED chars
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

// Task Handler
TaskHandle_t LCDTaskHandle;
QueueHandle_t Queue_dino = 0;
QueueHandle_t Queue_cac1 = 0;
QueueHandle_t Queue_cac2 = 0;

// Photon
const int photoPin0 = A0; // right
const int photoPin1 = A1; // left

const int delay_LED = 10;

typedef struct{
  int x = random(1,15);
  int y = random(0,1);
} coor;

int egg_idx = 0;
int egg_x[3] = {-1,-1,-1};
int egg_y[3] = {-1,-1,-1};
int egg_type[3] = {0};

// Keymap
char keymap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte colPins[4] = {9, 8, 7, 6};
byte rowPins[4] = {13, 12, 11, 10};

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

void displayTask(void *pvParameters){
  coor dino_c;
  coor cac1_c;
  coor cac2_c;
  int game_continue = 1; // 0,1
  for(;;){
    lcd.clear();

    xQueueReceive(Queue_dino, &dino_c, 0);
    xQueueReceive(Queue_cac1, &cac1_c, 0);
    xQueueReceive(Queue_cac2, &cac2_c, 0);
    //Serial.println(dino_c.x);
    //Serial.println(dino_c.y);

    lcd.setCursor(dino_c.x, dino_c.y);
    lcd.write(0);
    lcd.setCursor(cac1_c.x, cac1_c.y);
    lcd.write(1);
    lcd.setCursor(cac2_c.x, cac2_c.y);
    lcd.write(1);

    for(int i=0; i<egg_idx; i++){
      lcd.setCursor(egg_x[i], egg_y[i]);
      if(egg_type[i]==1){ // intact egg
        lcd.write(2);
      }
      else{ // broken egg
        lcd.write(3);
        game_continue = 0; // set flag
      }
    }

    if(game_continue==0){
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
    
    vTaskDelay(delay_LED); // stable the LEDs
  }
}

coor cactus_move(coor orig_coor, coor obj1, coor obj2){ // object_to_move, dodge_cactus, dodge_dino

  int dodge_x[2] = {obj1.x, obj2.x};
  int dodge_y[2] = {obj1.y, obj2.y};

  while(1){
    // Choose between up, down, right, left
    int dir = random(0, 4);
  
    // Move cactus
    if(dir==0){ // left
      if(orig_coor.x!=0){
      //if(orig_coor.x-1!=dodge_x[0] && orig_coor.y!=dodge_y[0]){
        if(orig_coor.x-1!=dodge_x[1] || orig_coor.y!=dodge_y[1]){
          orig_coor.x = orig_coor.x-1;
          break;
        }
      //}
      }
    }
    else if(dir==1){ // right
      if(orig_coor.x!=15){
      //if(orig_coor.x+1!=dodge_x[0] && orig_coor.y!=dodge_y[0]){
        if(orig_coor.x+1!=dodge_x[1] || orig_coor.y!=dodge_y[1]){
          orig_coor.x = orig_coor.x+1;
          break;
        }
      //}
      }
    }
    else if(dir==2){ // up
      if(orig_coor.y!=0){
      //if(orig_coor.x!=dodge_x[0] && orig_coor.y-1!=dodge_y[0]){
        if(orig_coor.x!=dodge_x[1] || orig_coor.y-1!=dodge_y[1]){
          orig_coor.y = 0;
          break;
        }
      //}
      }
    }
    else if(dir==3){ // down
      if(orig_coor.y!=1){
      //if(orig_coor.x!=dodge_x[0] && orig_coor.y+1!=dodge_y[0]){
        if(orig_coor.x!=dodge_x[1] || orig_coor.y+1!=dodge_y[1]){
          orig_coor.y = 1;
          break;
        }
      //}
      }
    }
    //Serial.print("Change\n");
  }

  for(int i=0; i<egg_idx; i++){
    if(orig_coor.x==egg_x[i] && orig_coor.y==egg_y[i]){
      //Gameover
      egg_type[i] = 0; // Change img
    }
  }
  
  return orig_coor;
}

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);
void controlTask(void *pvParameters){
  coor dino_c;
  coor cac1_c;
  coor cac2_c;
  for(;;){
    
    char key = myKeypad.getKey();

    // Move dino
    if(key=='2'){
      if(dino_c.y==1){
        dino_c.y = 0;
      }
    }
    else if(key=='8'){
      if(dino_c.y==0){
        dino_c.y = 1;
      }
    }
    else if(key=='4'){
      if(dino_c.x!=0){
        dino_c.x = dino_c.x-1;
      }
    }
    else if(key=='6'){
      if(dino_c.x!=15){
        dino_c.x = dino_c.x+1;
      }
    }
    else if(key=='5'){ //lay egg
      egg_x[egg_idx] = dino_c.x;
      egg_y[egg_idx] = dino_c.y;
      egg_type[egg_idx] = 1;
      egg_idx++;
    }

    // Move cactus
    if(key=='2' || key=='8' || key=='4' || key=='6'){
      cac1_c = cactus_move(cac1_c, cac2_c, dino_c);
      cac2_c = cactus_move(cac2_c, cac1_c, dino_c);
    }
    
    xQueueSend(Queue_dino, &dino_c, 0);
    xQueueSend(Queue_cac1, &cac1_c, 0);
    xQueueSend(Queue_cac2, &cac2_c, 0);
    //Serial.println(dino_c.x);
    //Serial.println(dino_c.y);

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

  Queue_dino = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int
  Queue_cac1 = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int
  Queue_cac2 = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int

  xTaskCreate(controlTask, "controlTask", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, "displayTask", 128, NULL, 1, &LCDTaskHandle);
  
  //Use memory carefully and set the size of task stack properly
  //Try and error
}

void loop(){

}