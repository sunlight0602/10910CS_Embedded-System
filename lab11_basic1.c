#include <Arduino_FreeRTOS.h> // for TaskHandle_t
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Queue.h>

LiquidCrystal_I2C lcd(0x3F,16,2); // 0x27 or 0x3F
QueueHandle_t Global_Queue_Handle = 0; //Global Handler

// LED chars
byte dinosaur[8]  = {0x07, 0x05, 0x06, 0x07, 0x14, 0x17, 0x0E, 0x0A};
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};

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
int speed = 4; // 2~15
int is_stop = 0;

int dino_x = 0;
int dino_y = 0;
int score = 0;

typedef struct{
  int x = 0;
  int y = 0;
} dino_coor;

// Keymap
char keymap[4][4] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte colPins[4] = {9, 8, 7, 6};
byte rowPins[4] = {13, 12, 11, 10};

void displayTask(void *pvParameters){
  dino_coor dino_c;
  
  for(;;){
    lcd.clear();

    xQueueReceive(Global_Queue_Handle, &dino_c, 0);
    //Serial.println(dino_c.x);
    //Serial.println(dino_c.y);

    lcd.setCursor(dino_c.x, dino_c.y);
    lcd.write(0);
    
    vTaskDelay(delay_LED); // stable the LEDs
  }
}

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);
void controlTask(void *pvParameters){
  dino_coor dino_c;
  for(;;){
    
    char key = myKeypad.getKey();
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
    
    xQueueSend(Global_Queue_Handle, &dino_c, 0);
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

  Global_Queue_Handle = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int

  xTaskCreate(controlTask, "controlTask", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, "displayTask", 128, NULL, 1, &LCDTaskHandle);
  
  //Use memory carefully and set the size of task stack properly
  //Try and error
}

void loop(){

}