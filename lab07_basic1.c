
// Set pin
const int buttonPin1 = 2;
const int buttonPin2 = 3;
const int buttonPin3 = 4;
const int ledPin1 = 5;
const int ledPin2 = 6;
const int ledPin3 = 7;

// States
int buttonState1;
int buttonState2;
int buttonState3;
int lastButtonState1 = LOW; // previous reading from the input pin
int lastButtonState2 = LOW;
int lastButtonState3 = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime2 = 0;  // the last time the output pin was toggled
unsigned long lastDebounceTime3 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // threshold debounce time to pass

// Button pressed time
int b1_time = 0;
int b2_time = 0;
int b3_time = 0;

// Timer state
int s = 0; // toogle between 0, 1
int s2 = 0;
int s3 = 0;

void set_timer(){
  noInterrupts(); // atomic access to timer reg.
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= (1 << WGM12); // turn on CTC mode
  TCCR1B |= (1<<CS12) | (1<<CS10); // 1024 prescaler
  OCR1A = 7812;  // give 0.5 sec at 16 MHz/1024
}

void cap_timer(){ // capture timer
  if (TIFR1 & (1 << OCF1A)) { // wait for time up
    if(buttonState1==1){
      b1_time++; 
      Serial.print("Button 1 Inc");
      Serial.print(b1_time);
      Serial.print("\n");
    }
    else{
      if(b1_time > 0){
        b1_time--;
        Serial.print("Button 1 Dec");
        Serial.print(b1_time);
        Serial.print("\n");
      }
    }
    
    if(s==1){ // to blink LED
      s = 0;
    }
    else{
      s = 1;
    }
  } 
}

void cap_timer2(){
  if (TIFR1 & (1 << OCF1A)){
    if(buttonState2){
      b2_time++; 
      Serial.print("Button 2 Inc");
      Serial.print(b2_time);
      Serial.print("\n");
    }
    else{
      if(b2_time > 0){
        b2_time--;
        Serial.print("Button 2 Dec");
        Serial.print(b2_time);
        Serial.print("\n");
      }
    }
    if(s2){s2 = 0;}
    else{s2 = 1;}    
  } 
}

void cap_timer3(){
  if (TIFR1 & (1 << OCF1A)) { // wait for time up
    if(buttonState3){
      b3_time++; 
      Serial.print("Button 3 Inc");
      Serial.print(b3_time);
      Serial.print("\n");
    }
    else{
      if(b3_time > 0){
        b3_time--;
        Serial.print("Button 3 Dec");
        Serial.print(b3_time);
        Serial.print("\n");
      }
    }

    if(s3){s3 = 0;}
    else{s3 = 1;}
  } 
}

void clear_timer(){
  if(TIFR1 & (1 << OCF1A)){ // wait for time up
    TIFR1 = (1<<OCF1A); // clear overflow flag
  }
}

void btn_control(int reading){
  if (reading != lastButtonState1) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState1) {
      buttonState1 = reading;
    }
  }

  lastButtonState1 = reading;
}

void btn_control2(int reading){
  if (reading != lastButtonState2) {
    // reset the debouncing timer
    lastDebounceTime2 = millis();
  }
//  current time - last time > delayTime
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState2) {//buttonState2 whether change state or not
      buttonState2 = reading;
    }
  }

  lastButtonState2 = reading;
}

void btn_control3(int reading){
  if (reading != lastButtonState3) {
    // reset the debouncing timer
    lastDebounceTime3 = millis();
  }

  if ((millis() - lastDebounceTime3) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState3) {
      buttonState3 = reading;
    }
  }

  lastButtonState3 = reading;
}

void led_control(){
  if(buttonState1==1){ // button is pressed
    digitalWrite(ledPin1, HIGH);
  }
  else{
    if(b1_time > 0){
      switch(s){
        case 1:
          digitalWrite(ledPin1, HIGH); // blink on
          break;
        case 0:
          digitalWrite(ledPin1, LOW); // blink off
          break;
      }
    }
    else{
      s = 0;
      digitalWrite(ledPin1, LOW);
    }
  }
}

void led_control2(){
  if(buttonState2==1){
    digitalWrite(ledPin2, HIGH);
  }
  else{
    if(b2_time > 0){
      switch(s2){
        case 1:
          digitalWrite(ledPin2, HIGH);
          break;
        case 0:
          digitalWrite(ledPin2, LOW);
          break;
      }
    }
    else{
      s2 = 0;
      digitalWrite(ledPin2, LOW);
    }
  }
}

void led_control3(){
  if(buttonState3==1){ 
    digitalWrite(ledPin3, HIGH);
  }
  else{
    if(b3_time > 0){
      switch(s3){
        case 1:
          digitalWrite(ledPin3, HIGH);
          break;
        case 0:
          digitalWrite(ledPin3, LOW);
          break;
      }
    }
    else{
      s3 = 0;
      digitalWrite(ledPin3, LOW);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);
  pinMode(buttonPin3, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);

  set_timer();
  interrupts(); // enable all interrupts

  // initialize
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  Serial.print("Test\n");
}

void loop() {
  int read_button1 = digitalRead(buttonPin1);
  int read_button2 = digitalRead(buttonPin2);
  int read_button3 = digitalRead(buttonPin3);

  // tiktok s, check if pressed
  cap_timer();
  cap_timer2();
  cap_timer3();
  clear_timer();

  // handle debounce, pass down button state
  btn_control(read_button1);
  btn_control2(read_button2);
  btn_control3(read_button3);
  
  // on, blink led
  led_control();
  led_control2();
  led_control3();
}