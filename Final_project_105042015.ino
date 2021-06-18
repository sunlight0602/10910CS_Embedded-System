#include <Arduino_FreeRTOS.h> // for TaskHandle_t
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Queue.h>

// Buzzers
const int buzzer1 = 2;
const int buzzer2 = 3;
#define Do 523
#define Do_s 554
#define Re 587
#define Re_s 622
#define Mi 659
#define Fa 698
#define Fa_s 740
#define So 784
#define So_s 831
#define La 880
#define La_s 932
#define Si 988
int melody[12] = {Do, Do_s, Re, Re_s, Mi, Fa, Fa_s, So, So_s, La, La_s, Si};

// Ultrasonic 
const int trigPin = 5;
const int echoPin = A2;
int distance = 0;
int duration = 0;
int ignore_photodet = 0; // 0,1

// LCD chars
LiquidCrystal_I2C lcd(0x3F,16,2); // 0x27 or 0x3F
byte cactus[8] = {0x04, 0x05, 0x15, 0x15, 0x16, 0x0C, 0x04, 0x04};
byte err_dragon[8]  = {B00111, B00101, B00110, B00111, B10100, B10111, B01110, B01010};
byte egg[8] = {0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000};
byte broken[8] = {B00100, B10101, B01110, B11111, B11111, B01110, B10101, B00100};

// Task Handler
TaskHandle_t LCDTaskHandle;
QueueHandle_t Queue_dino = 0;
QueueHandle_t Queue_cac1 = 0;
QueueHandle_t Queue_cac2 = 0;

// RGB
//const int ledPinR = 0;
//const int ledPinG = 1;
//const int ledPinB = 4;

// Photon
const int photoPin0 = A0; // right
const int photoPin1 = A1; // left

const int delay_LED = 10;

// Variables
int ans_idx = 0;
int total_idx = 0;
char ans_note[3] = {'_','_','_'};
char ans_sig[3] = {'_','_','_'};
int note1 = 999;
int note2 = 999;
int difficulty = 0; // 0,1

int game_state = 0; // 0:welcome, 1:answer, 2:correct/wrong

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
  for(;;){
    lcd.clear();

    if(game_state==0){
      
      lcd.setCursor(0, 0);
      lcd.print(F("Tune Trainer!"));
      lcd.setCursor(0, 1);
      lcd.print(F("Select mode: "));
      lcd.setCursor(13, 1);
      lcd.print(difficulty+1);
    }
    else if(game_state==1){
      lcd.setCursor(0, 0);
      lcd.print(F("Your answer:"));
  
      for(int i=0; i<total_idx; i++){
        if(i==0 || i==1){
          lcd.setCursor(0, 1);
          lcd.print(ans_note[0]);
          lcd.setCursor(1, 1);
          lcd.print(ans_sig[0]);
          if(i==1){
            lcd.setCursor(3, 1);
            lcd.print(F("__"));
          }
        }
        else if(i==2 || i==3){
          lcd.setCursor(3, 1);
          lcd.print(ans_note[1]);
          lcd.setCursor(4, 1);
          lcd.print(ans_sig[1]);
        }
      }
    }
    else if(game_state==2){
      int correct = correctOrWrong();
      if(correct){ // correct
        
        lcd.setCursor(0, 0);
        lcd.print(ans_note[0]);
        lcd.setCursor(1, 0);
        lcd.print(ans_sig[0]);
        if(difficulty==1){
          lcd.setCursor(3, 0);
          lcd.print(ans_note[1]);
          lcd.setCursor(4, 0);
          lcd.print(ans_sig[1]);
        }
        lcd.setCursor(3, 1);
        lcd.print(F("is Correct!"));
      }
      else{ // wrong
        
        lcd.setCursor(0, 0);
        lcd.print(F("Wrong! Answer:"));

        lcd.setCursor(0, 1);
        if(note1==0){
          lcd.print(F("1-"));
        }
        else if(note1==1){
          lcd.print(F("1#or2*"));
        }
        else if(note1==2){
          lcd.print(F("2-"));
        }
        else if(note1==3){
          lcd.print(F("2#or3*"));
        }
        else if(note1==4){
          lcd.print(F("3-"));
        }
        else if(note1==5){
          lcd.print(F("4-"));
        }
        else if(note1==6){
          lcd.print(F("4#or5*"));
        }
        else if(note1==7){
          lcd.print(F("5-"));
        }
        else if(note1==8){
          lcd.print(F("5#or6*"));
        }
        else if(note1==9){
          lcd.print(F("6-"));
        }
        else if(note1==10){
          lcd.print(F("6#or7*"));
        }
        else if(note1==11){
          lcd.print(F("7-"));
        }

        if(difficulty==1){
          lcd.setCursor(7, 1);
          if(note2==0){
            lcd.print(F("1-"));
          }
          else if(note2==1){
            lcd.print(F("1#or2*"));
          }
          else if(note2==2){
            lcd.print(F("2-"));
          }
          else if(note2==3){
            lcd.print(F("2#or3*"));
          }
          else if(note2==4){
            lcd.print(F("3-"));
          }
          else if(note2==5){
            lcd.print(F("4-"));
          }
          else if(note2==6){
            lcd.print(F("4#or5*"));
          }
          else if(note2==7){
            lcd.print(F("5-"));
          }
          else if(note2==8){
            lcd.print(F("5#or6*"));
          }
          else if(note2==9){
            lcd.print(F("6-"));
          }
          else if(note2==10){
            lcd.print(F("6#or7*"));
          }
          else if(note2==11){
            lcd.print(F("7-"));
          }
        }
      }
    }

    vTaskDelay(delay_LED); // stable the LEDs
  }
}

void restartGame(){
  for(int i=0; i<3; i++){
    ans_note[i] = '_';
    ans_sig[i] = '_';
  }
  ans_idx = 0;
  total_idx = 0;
  note1 = 999;
  note2 = 999;
  
  return ;
}

int correctOrWrong(){
  if(difficulty==0){
    if(melody[note1]==Do){
      if(ans_note[0]=='1' && ans_sig[0]=='-'){
        return 1;
      }
    }
    else if(melody[note1]==Do_s){
      if(ans_note[0]=='1' && ans_sig[0]=='#' || ans_note[0]=='2' && ans_sig[0]=='*'){
        return 1;
      }
    }
    else if(melody[note1]==Re){
      if(ans_note[0]=='2' && ans_sig[0]=='-'){
        return 1;
      }
    }
    else if(melody[note1]==Re_s){
      if(ans_note[0]=='2' && ans_sig[0]=='#' || ans_note[0]=='3' && ans_sig[0]=='*'){
        return 1;
      }
    }
    else if(melody[note1]==Mi){
      if(ans_note[0]=='3' && ans_sig[0]=='-'){
        return 1;
      }
    }
    else if(melody[note1]==Fa){
      if(ans_note[0]=='4' && ans_sig[0]=='-'){
        return 1;
      }
    }
    else if(melody[note1]==Fa_s){
      if(ans_note[0]=='4' && ans_sig[0]=='#' || ans_note[0]=='5' && ans_sig[0]=='*'){
        return 1;
      }
    }
    else if(melody[note1]==So){
      if(ans_note[0]=='5' && ans_sig[0]=='-'){
        return 1;
      }
    }
    else if(melody[note1]==So_s){
      if(ans_note[0]=='5' && ans_sig[0]=='#' || ans_note[0]=='6' && ans_sig[0]=='*'){
        return 1;
      }
    }
    else if(melody[note1]==La){
      if(ans_note[0]=='6' && ans_sig[0]=='-'){
        return 1;
      }
    }
    else if(melody[note1]==La_s){
      if(ans_note[0]=='6' && ans_sig[0]=='#' || ans_note[0]=='7' && ans_sig[0]=='*'){
        return 1;
      }
    }
    else if(melody[note1]==Si){
      if(ans_note[0]=='7' && ans_sig[0]=='-'){
        return 1;
      }
    }
  }
  else if(difficulty==1){
    int correct_flag[2] = {0, 0};
    if(melody[note1]==Do){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='1' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Do_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='1' && ans_sig[i]=='#' || ans_note[i]=='2' && ans_sig[i]=='*'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Re){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='2' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Re_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='2' && ans_sig[i]=='#' || ans_note[i]=='3' && ans_sig[i]=='*'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Mi){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='3' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Fa){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='4' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Fa_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='4' && ans_sig[i]=='#' || ans_note[i]=='5' && ans_sig[0]=='*'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==So){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='5' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==So_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='5' && ans_sig[i]=='#' || ans_note[i]=='6' && ans_sig[i]=='*'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==La){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='6' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==La_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='6' && ans_sig[i]=='#' || ans_note[i]=='7' && ans_sig[i]=='*'){
          correct_flag[0] = 1;
          break;
        }
      }
    }
    else if(melody[note1]==Si){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='7' && ans_sig[i]=='-'){
          correct_flag[0] = 1;
          break;
        }
      }
    }

    // Second note
    if(melody[note2]==Do){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='1' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Do_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='1' && ans_sig[i]=='#' || ans_note[i]=='2' && ans_sig[i]=='*'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Re){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='2' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Re_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='2' && ans_sig[i]=='#' || ans_note[i]=='3' && ans_sig[i]=='*'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Mi){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='3' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Fa){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='4' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Fa_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='4' && ans_sig[i]=='#' || ans_note[i]=='5' && ans_sig[0]=='*'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==So){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='5' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==So_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='5' && ans_sig[i]=='#' || ans_note[i]=='6' && ans_sig[i]=='*'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==La){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='6' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==La_s){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='6' && ans_sig[i]=='#' || ans_note[i]=='7' && ans_sig[i]=='*'){
          correct_flag[1] = 1;
          break;
        }
      }
    }
    else if(melody[note2]==Si){
      for(int i=0; i<2; i++){
        if(ans_note[i]=='7' && ans_sig[i]=='-'){
          correct_flag[1] = 1;
          break;
        }
      }
    }

    // Determine
    if(correct_flag[0]==1 && correct_flag[1]==1){
      return 1;
    }
  }
  
  return 0;
}

Keypad myKeypad = Keypad(makeKeymap(keymap), rowPins, colPins, 4, 4);
void controlTask(void *pvParameters){
  
  for(;;){
    
    char key = myKeypad.getKey();

    // Proceed game_state
    if(key=='D'){
      if(game_state==0){
        if(difficulty==0){
          int i = random(12);
          note1 = i;
        }
        else if(difficulty==1){
          int i = random(12);
          note1 = i;
          int j = random(12);
          note2 = j;
        }
        game_state = 1;
      }
      else if(game_state==1){
        game_state = 2;
      }
      else if(game_state==2){
        restartGame();
        game_state = 0;
      }
    }

    // Input answer
    if(key=='1' || key=='2' || key=='3'|| key=='4'|| key=='5'|| key=='6'\
    || key=='7'|| key=='*'|| key=='0'|| key=='#'){
      Serial.print(F("Keypad "));
      Serial.println(key);
      
      if(total_idx%2==0){
        ans_note[ans_idx] = key;
        total_idx++;
      }
      else{
        if(key=='0'){
          ans_sig[ans_idx] = '-';
        }
        else{
          ans_sig[ans_idx] = key;
        }
        total_idx++;
        ans_idx++;
      }
    }

    vTaskDelay(delay_LED);
  }
}

#define PAUSE 1000     // length of pause between notes
#define REST_COUNT 185
void rest(long duration) {
  for (int j = 0; j < REST_COUNT; j++) {
    delayMicroseconds(duration);  
  }                                  
}
void play2Tones(int tone1, int tone2, long duration){
  byte s1, s2; // state of the buzzers
  long sum1, sum2, cur, next, n1, n2;
  cur = next = sum1 = sum2 = 0;

  // Init buzzers
  s1 = s2 = LOW;
  digitalWrite(buzzer1, s1);
  digitalWrite(buzzer2, s2);
  
//    duration -= PAUSE;
  if(tone1==Do){
    tone1 = 3822;
  }
  else if(tone1==Do_s){
    tone1 = 3608;
  }
  else if(tone1==Re){
    tone1 = 3405;
  }
  else if(tone1==Re_s){
    tone1 = 3214;
  }
  else if(tone1==Mi){
    tone1 = 3034;
  }
  else if(tone1==Fa){
    tone1 = 2863;
  }
  else if(tone1==Fa_s){
    tone1 = 2703;
  }
  else if(tone1==So){
    tone1 = 2551;
  }
  else if(tone1==So_s){
    tone1 = 2408;
  }
  else if(tone1==La){
    tone1 = 2273;
  }
  else if(tone1==La_s){
    tone1 = 2145;
  }
  else if(tone1==Si){
    tone1 = 2025;
  }
  
  if(tone2==Do){
    tone2 = 3822;
  }
  else if(tone2==Do_s){
    tone2 = 3608;
  }
  else if(tone2==Re){
    tone2 = 3405;
  }
  else if(tone2==Re_s){
    tone2 = 3214;
  }
  else if(tone2==Mi){
    tone2 = 3034;
  }
  else if(tone2==Fa){
    tone2 = 2863;
  }
  else if(tone2==Fa_s){
    tone2 = 2703;
  }
  else if(tone2==So){
    tone2 = 2551;
  }
  else if(tone2==So_s){
    tone2 = 2408;
  }
  else if(tone2==La){
    tone2 = 2273;
  }
  else if(tone2==La_s){
    tone2 = 2145;
  }
  else if(tone2==Si){
    tone2 = 2025;
  }
  tone1 >>= 1;
  tone2 >>= 1;
  while(cur < duration){
    next = min( min(sum1+tone1, sum2+tone2), duration ); // next = smallest among 3
    delayMicroseconds(next-cur);
    
    if(sum1 + tone1 == next){
      s1 = (s1==HIGH)? LOW: HIGH; // s1 high->low, low->high
      digitalWrite(buzzer1, s1);
      sum1 += tone1;
    }
    if(sum2 + tone2 == next){
      s2 = (s2==HIGH)? LOW: HIGH; // s2 high->low, low->high
      digitalWrite(buzzer2, s2);
      sum2 += tone2;
    }
    cur = next;
  }
  delayMicroseconds(PAUSE);
}

void play_Tones(int melody1, int melody2){
  tone(buzzer1, melody1);
  delay(10);
  noTone(buzzer1);
  tone(buzzer2, melody2);
  delay(10);
  noTone(buzzer2);

  return ;
}

//int DURATION = 840000;
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
    if(distance > 15 || game_state==0){
      ignore_photodet = 1;
      noTone(buzzer1);
      noTone(buzzer2);
    }
    else if(distance<=15 && distance>=0){
      ignore_photodet = 0;
      if(note1 != 999 && difficulty==0){
        tone(buzzer1, melody[note1]);
      }
      if(note1 != 999 && note2 != 999 && difficulty==1){
        Serial.print(F("buzzer2\n"));
//        tone(buzzer2, melody[note2]);
          play2Tones(melody[note1], melody[note2], 840000);//beats1[0] * 4 * TEMPO);
//          play_Tones(melody[note1], melody[note2]);
      }
    }

    Serial.print(difficulty);
    Serial.print(F(" | "));
    Serial.print(note1);
    Serial.print(F(" | "));
    Serial.print(note2);
    Serial.print(F(" | "));
    Serial.print(distance);
//    Serial.print(" | ");
//    Serial.print(ignore_photodet);
    Serial.print("\n");

    delay(300);
  }
}

void leftTask(void *pvParameters){
  for(;;){
    int photo_right = analogRead(photoPin0);
    int photo_left = analogRead(photoPin1);

    if(game_state==0){
      if(photo_left < photo_right-150){ //Left photoresistor dark for deceleration
        if(difficulty!=0){
          difficulty--;
        }
      }
      else if(photo_right < photo_left-75){ //Right photoresistor dark for acceleration
        if(difficulty!=1){
          difficulty++;
        }
      }
    }

    
    vTaskDelay(delay_LED);
  }
}

void setup(){
  
  Serial.begin(9600); // select 9600 baud
  Serial.print(F("Start\n"));

  // Buzzer
  pinMode(buzzer1, OUTPUT);
  pinMode(buzzer2, OUTPUT);

  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // LCD
  lcd.init(); // initialize LCD
  lcd.backlight(); // open LCD backlight
  lcd.createChar(0, err_dragon);
  lcd.createChar(1, cactus);
  lcd.createChar(2, egg);
  lcd.createChar(3, broken);

  // RGB
//  pinMode(ledPinR, OUTPUT);
//  pinMode(ledPinG, OUTPUT);
//  pinMode(ledPinB, OUTPUT);
//  analogWrite(ledPinR, 0);
//  analogWrite(ledPinG, 0);
//  digitalWrite(ledPinR, LOW);
//  digitalWrite(ledPinG, LOW);
//  analogWrite(ledPinB, 0);
  
  //Queue_dino = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int
  //Queue_cac1 = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int
  //Queue_cac2 = xQueueCreate(3, sizeof(int)*2); // Create a queue of 3 int

  randomSeed(analogRead(A3));

  xTaskCreate(controlTask, "controlTask", 64, NULL, 1, NULL); // Keypad
  xTaskCreate(t_sonic, "t_sonic", 128, NULL, 1, NULL);
  xTaskCreate(displayTask, "displayTask", 100, NULL, 1, &LCDTaskHandle);

  xTaskCreate(leftTask, "leftTask", 64, NULL, 1, NULL);
  
  //Use memory carefully and set the size of task stack properly
  //Try and error
}

void loop(){
}
