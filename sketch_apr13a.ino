#include <IRremote.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#define True 1
#define False 0
#define Active True
#define Inactive False

#define IR_NONE 0
#define IR_ONE 12495
#define IR_TWO 6375
#define IR_THREE 31365
#define IR_FOUR 4335
#define IR_B -23971
#define IR_V -7651
#define IR_HOLD -1

// ---------- Declaration ----------
int N;

// Pin
int cdsPins[4] = {A0, A1, A2, A3};
int ledPins[4] = {13, 12, 11, 10};
int IR_pin = 8;

// LCD
LiquidCrystal lcd(7,6,5,4,3,2);

// IR
IRrecv irrecv(IR_pin);
int currentHoldingButton = IR_NONE;
// SERVO
Servo myservo;
Servo yourservo;
// Game settings
int maximumConcurrentTarget = 2;
int targetDuration = 3;
int brightnessThreshold = 900;
int gameMaxTime = 60;

// Game
int isGameInProgress = False;
int gameStartTime = 0;
int score = 0;

long int target_key_match[4] = {IR_ONE, IR_TWO, IR_THREE, IR_FOUR};
int targetStatus[4] = {Inactive, Inactive, Inactive, Inactive};
int targetActivatedTime[4] = {0, 0, 0, 0};
int targetCount = 0;

// -------------------- Declaration --------------------
void startGame();
void endGame();
void handleGame();
void addRandomTarget();
int detectHitTarget(int pin);

void configure();
void configurePinMode();
void configureServo();

void increaseScore();
void resetScore();
String scoreInString();
void printCurrentScore();

int getBrightness(int pin);

void turnOnLed(int pin);
void turnOffLed(int pin);

void configureIr();
int receiveIrSignal();
void handleIrSignal(int code);


// -------------------- Main --------------------
void setup() {
  configure();
  configurePinMode();
  configureIr();
  configureServo();
}

void loop() {
  int currentTime = millis();

  // Get IR signal
  int code = receiveIrSignal();
  handleIrSignal(code);

  // Game running
  if (isGameInProgress == true) {
    handleGame(currentTime);
    if (currentTime > gameStartTime && currentTime - gameStartTime > gameMaxTime * 1000.0) {
      endGame();
    }
  }
}

// -------------------- Setup --------------------
void configure() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  N = sizeof(cdsPins) / sizeof(cdsPins[0]);
}

void configurePinMode() {
  for(int i = 0; i < N; i++) {
    int currentPin = ledPins[i];
    pinMode(currentPin, OUTPUT);
  }
  
  for(int i = 0; i < N; i++) {
    int currentPin = cdsPins[i];
    pinMode(currentPin, INPUT);
  }
  pinMode(IR_pin, INPUT);
}

// -------------------- Game --------------------
void startGame() {
  Serial.println("Game starting!");
  myservo.write(170);
  yourservo.write(0);
  lcd.clear();
  lcd.begin(16, 2); 
  lcd.print("Welcome!");
  lcd.setCursor(0,1);
  lcd.print("Starting game...");

  isGameInProgress = True;
  resetScore();
  gameStartTime = millis();
}

void endGame() {
  isGameInProgress = False;

  for(int i = 0; i < N; i++) {
    turnOffLed(ledPins[i]);
  }
  Serial.println("Game ended!");
  Serial.print("You scored ");
  Serial.print(score);
  Serial.print("!");
  myservo.write(130);
  yourservo.write(30);

  String string = scoreInString();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("End! You scored:");
  lcd.setCursor(0,1);
  lcd.print(string);
}

void handleGame(int currentTime) {
   // Detect if target is hit
   for (int i = 0; i < N; i++){
     if (targetStatus[i] == Active) {
       if (detectHitTarget(i)) {
        targetStatus[i] = Inactive;
        targetCount -= 1;
        turnOffLed(ledPins[i]);
        increaseScore();
        lcd.clear();
        printCurrentScore();  
      }
    }
   }
   
  // Phase out unhit targets 
  for (int i = 0; i < N; i++) {
    if (targetStatus[i] == Active && currentTime - targetActivatedTime[i] > targetDuration * 1000) {
      targetStatus[i] = Inactive;
      targetCount -= 1;
      turnOffLed(ledPins[i]);
    }
   }  

  // Serial.print("HOLDING BUTTON : ");
  // Serial.println(currentHoldingButton);

   // Add a new target if necessary
   if (targetCount < maximumConcurrentTarget) {
  addRandomTarget(currentTime);
   }
}

void addRandomTarget(int currentTime) {
  int target;
  do {
    target = random(0, N);  
  } while(targetStatus[target] == Active);

  targetStatus[target] = Active;
  targetActivatedTime[target] = currentTime;
  targetCount += 1;
  turnOnLed(ledPins[target]);
}

int detectHitTarget(int targetIndex) {
   //by Laser
  if(getBrightness(cdsPins[targetIndex]) > brightnessThreshold) {
    irrecv.resume(); 
    return True;
  }
   //by IR
   else if (currentHoldingButton == target_key_match[targetIndex]) {
    currentHoldingButton = IR_NONE;
    return True;
  } else {
    return False;
  }
}

// -------------------- Score --------------------
void increaseScore() {
  score += 1;
}

void resetScore() {
  score = 0;
}

String scoreInString() {
  return String(score);
}

void printCurrentScore() {
  String string = scoreInString();
  lcd.setCursor(0,0);
  lcd.print("Score");
  lcd.setCursor(0,1);
  lcd.print(string);
}

// -------------------- CDS --------------------
int getBrightness(int pin) {
  return analogRead(pin);
}

// -------------------- LED --------------------
void turnOnLed(int pin) {
  digitalWrite(pin, HIGH);
  // analogWrite(pin, 50);
}
void turnOffLed(int pin) {
  digitalWrite(pin, LOW);
  // analogWrite(pin, 0);
}

// -------------------- IR --------------------
void configureIr() {
  irrecv.enableIRIn();
}
// -------------- Servo ----------------
void configureServo(){
  Serial.begin(9600);
  myservo.attach(A4);
  yourservo.attach(A5);
  myservo.write(130);
  yourservo.write(30);
}
int receiveIrSignal() {
  decode_results results;
  if (irrecv.decode(&results)) {
    int code = results.value & 0xFFFF;
      irrecv.resume();
    return code;
  } else {
    return IR_NONE;
  }
}

void handleIrSignal(int code) {
  switch (code) {
    case IR_ONE:
    Serial.println("ONE");
    currentHoldingButton = IR_ONE;
    break;
    case IR_TWO:
    Serial.println("TWO");
    currentHoldingButton = IR_TWO;
    break;
    case IR_THREE: 
    Serial.println("THREE");
    currentHoldingButton = IR_THREE;
    break;
    case IR_FOUR:
    Serial.println("FOUR");
    currentHoldingButton = IR_FOUR;
    break;
    case IR_B:
    Serial.println("B");
    currentHoldingButton = IR_B;
    if (isGameInProgress == False) {
      startGame();
    }
    break;
    case IR_V:
    Serial.println("V");
    currentHoldingButton = IR_V;
    if (isGameInProgress == True) {
      endGame();
    }
    break;
    case IR_HOLD:
    Serial.println("HOLD");
    break;
    case IR_NONE: // None
    break;
    default: 
    Serial.print("Unknown code : ");
    Serial.println(code);
    break;
  }
}
