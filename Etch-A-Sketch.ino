//  
//  Addy Aque
//
//  August 9, 2024
//
//  With an MPU6050 accelerometer, a TFT_HX8357 LCD Screen, Two Rotary Encoders, and several Push-Buttons, this program
//    functions as an Etch-A-Sketch. There are two game modes:
//    1) Etch-A-Sketch: Using knobs to move the cursor, a button which
//      allows fo rerasing, as well as an the accelerometer which erases the screen.
//    2) Tilt-A-Sketch: By tilting the MPU6050, the angle determines the direction of movement, and it erases with a
//      harder thud downwards against a flat surface.
//    3) DOOM 1993 (in-progress): I plan to make DOOM a playable game using the inputs.
//
//***********
// Libraries
//***********
// For the accelerometer, I used a library by Tockn: https://github.com/tockn/MPU6050_tockn
// For the TFT LCD Screen, I used a library by banggood: http://www.banggood.com/3_0-Inch-320-X-480-TFT-LCD-Display-Module-Support-Arduino-Mega2560-p-963573.html

// Accelerometer
#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

long timer = 0;

// TFT screen
#define BLACK 0x0000
#define WHITE 0xFFFF
#define GREY  0x5AEB
#define TFT_LIGHTPINK 0xF4D3

#include <TFT_HX8357.h> // Hardware-specific library

// Rotary encoder pins
#define CLK A1
#define DT A0
#define CLK2 A2
#define DT2 A3

TFT_HX8357 tft = TFT_HX8357(320,480); // Invoke custom library

// Setting the bounds of the screen
int16_t h = 320;
int16_t w = 480;


// Value to determine size of PX readings array
const int numReadingsPX = 5;

int readingsPX[numReadingsPX];  // the readings from the analog input
int readIndexPX = 0;          // the index of the current reading
int totalPX = 0;              // the tunning total
int averagePX = 0;            // the average

int potX = A0;

// Value to determine size of PY readings array
const int numReadingsPY = 5;

int readingsPY[numReadingsPY];  // the readings from the analog input
int readIndexPY = 0;          // the index of the current reading
int totalPY = 0;              // the tunning total
int averagePY = 0;            // the average

int potY = A2;

// Create values for X and Y coords
int xCoord0 = 240;
int xCoord1 = 240;  //replace with variable
int yCoord0 = 160;
int yCoord1 = 160;  //replace with variable

// Push Buttons
const int b1 = A4;
const int b2 = A5;
const int b3 = A6;
const int b4 = A7;

int b1Count = 1;
int b2Count = 1;

bool b1Bool = false;
bool b2Bool = false;

int button1 = 3;

// Random variable
int z = 0;
int x = 0;

// Rotary encoder
int counterX = 240;
int counterY = 160;
int curStateCLK;
int lastStateCLK;
int curStateCLK2;
int lastStateCLK2;

// Angle Etch
// Creating array
int angArray[4];

// Creating objects for angArray
int oldXA;
int oldYA;

int newXA;
int newYA;

// Tolerances Array
int tolerances[4] = {7, -7, -7, 7};

// Creating variables for the pixel plotting
int xVal;
int yVal;
int tempVal;

void setup() {
  // Accelerometer
  Serial.begin(9600);
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  mpu6050.update();
  newXA = int(mpu6050.getAccAngleX());
  newYA = int(mpu6050.getAccAngleY());
  angArray[2] = int(newXA);
  angArray[3] = int(newYA);
  angArray[0] = angArray[2];
  angArray[1] = angArray[3];

  // TFT screen
  tft.begin();
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  xVal = 240;
  yVal = 160;

  tft.setTextColor(WHITE, BLACK);

  displayHome();

  // Rotary encoder

  // Setting them as inputs
  pinMode(CLK, INPUT);
  pinMode(DT,INPUT);
  pinMode(CLK2, INPUT);
  pinMode(DT2,INPUT);
  // Read initial stae of CLK
  lastStateCLK = digitalRead(CLK);
  lastStateCLK2 = digitalRead(CLK2);

}

void loop() {  
  // If button 1 is pressed, run etch a sketch
  if (digitalRead(b1) == HIGH) {
    delay(100);
    button1 = checkB1();
    if (button1 == 1) {
      delay(100);
      runTiltA();
      delay(750);
    }
    else {
      delay(750);
      runEtchA();
      delay(750);
    }
  }

  // If button 2 is pressed, it will display the data from the MPU6050
  if(digitalRead(b2) == HIGH) {
    delay(750);
    runAccelData();
    delay(750);
  }
}

//*************************
//  Home Screen Functions
//*************************

//  displayHome() 
//  Clears screen, gives instructions on how to use the Etch-A-Sketch,
//    prints my name.
//
//  Returns nothing.
//
void displayHome() {
  tft.setCursor(20, 10);
  tft.setTextFont(1);  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Welcome!");
  tft.println();
  tft.print("  "); tft.println("For etch-a-sketch, press B1.");
  tft.print("  "); tft.println("For angle-a-sketch, hold B1.");
  tft.print("  "); tft.println("For data, press B2.");
  tft.print("  "); tft.println("To reurn here, hold B1");

  tft.setCursor(320, 10);
  tft.setTextColor(TFT_LIGHTPINK, TFT_BLACK);
  tft.println("Created By: Addison Aque");
}

//***************************
//  Accelerometer Functions
//***************************

void runAccelData() {

  tft.fillScreen(TFT_BLACK); 

  // EtchA will run until button is pressed again
  while (digitalRead(b1) != HIGH){
    send6050Data();
    delay(100);
  }
  tft.fillScreen(TFT_BLACK);
  displayHome();
}

// Send the accelerometer data to the screen
void send6050Data() {
  mpu6050.update();
  tft.setTextColor(WHITE, BLACK);
  tft.setCursor(100,0);
  tft.setTextFont(1); tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.print("Temp: "); tft.print(mpu6050.getTemp());
  tft.setCursor(100, 20);
  tft.print("\tAccX: "); tft.print(mpu6050.getAccX());
  tft.setCursor(100,40);
  tft.print("\taccY : ");tft.print(mpu6050.getAccY());
  tft.setCursor(100,60);
  tft.print("\taccZ : ");tft.println(mpu6050.getAccZ());
  
  tft.setCursor(100,80);
  tft.print("gyroX : ");tft.print(mpu6050.getGyroX());
  tft.setCursor(100,100);
  tft.print("\tgyroY : ");tft.print(mpu6050.getGyroY());
  tft.setCursor(100,120);
  tft.print("\tgyroZ : ");tft.println(mpu6050.getGyroZ());
  
  tft.setCursor(100,140);
  tft.print("accAngleX : ");tft.print(mpu6050.getAccAngleX());
  tft.setCursor(100,160);
  tft.print("\taccAngleY : ");tft.println(mpu6050.getAccAngleY());
  
  tft.setCursor(100,180);
  tft.print("gyroAngleX : ");tft.print(mpu6050.getGyroAngleX());
  tft.setCursor(100,200);
  tft.print("\tgyroAngleY : ");tft.print(mpu6050.getGyroAngleY());
  tft.setCursor(100,220);
  tft.print("\tgyroAngleZ : ");tft.println(mpu6050.getGyroAngleZ());
  
  tft.setCursor(100,240);
  tft.print("angleX : ");tft.print(mpu6050.getAngleX());
  tft.setCursor(100,260);
  tft.print("\tangleY : ");tft.print(mpu6050.getAngleY());
  tft.setCursor(100,280);
  tft.print("\tangleZ : ");tft.println(mpu6050.getAngleZ());
}

//  checkShake()
//  If shaken up and down hard enough, coursor goes to middle and screen is wiped
//
//  Returns Nothing
//
void checkShake() {
  mpu6050.update();

  if ((mpu6050.getAccY() >= 1.9) || (mpu6050.getAccY() <=-1.9)) {
    counterX = 240;
    counterY = 160;
    xVal = 240;
    yVal = 160;
    tft.fillScreen(TFT_BLACK);
    delay(500);
  }
}

//***************************
//  Etch-A-Sketch Functions
//***************************

//  runEtchA()
//  Draws onto the LCD using several of the functions below. Uses the
//    rotary encoders to chenge the drawing location.
//
//  Returns nothing.
//
void runEtchA() {
  // Clears screen
  delay(50);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2); tft.setTextColor(WHITE, BLACK);
  
  // EtchA will run until button is pressed
  x = 0;
  while (x != 1){
    x = checkB1();
    checkShake();
    xRotEn();
    yRotEn();
    sketchRot(x);
  }

  // Resets b1Bool to FALSE
  b1Bool = false;

  //clears screen and brings back home
  tft.fillScreen(TFT_BLACK);
  displayHome();
}

//  xRotEn()
//  Keeps track of the movement of the rotary encoder for X-axis.
//
//  Returns nothing.
//
void xRotEn() {
  // Reads the state of CLK
  curStateCLK = digitalRead(CLK);

  // If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (curStateCLK != lastStateCLK  && curStateCLK == 1){

		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		
    if (counterX <= 0) {
    counterX = 1;
    }
    else if (counterX >= 480) {
      counterX = 479;
    }
    else if (digitalRead(DT) != curStateCLK) {
			counterX += 2;
			//currentDir = "CCW";
		} 
    else {
			// Encoder is rotating CW so increment
			counterX -= 2;
			//currentDir = "CW";
		}
	}

  //Remember last CLK state
  lastStateCLK = curStateCLK;

  // tft.setCursor(20, 100);
  // tft.setTextFont(1);  tft.setTextSize(1);
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // tft.print("Rotary Encoder Value:"); tft.print(counterX);
}

//  yRotEn()
//  Keeps track of the movement of the rotary encoder for Y-axis.
//
//  Returns nothing.
//
void yRotEn() {
  //read the state of CLK
  curStateCLK2 = digitalRead(CLK2);

  // If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (curStateCLK2 != lastStateCLK2  && curStateCLK2 == 1){

		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT2) != curStateCLK2) {
			counterY += 2;
			//currentDir = "CCW";
		} else {
			// Encoder is rotating CW so increment
			counterY -= 2;
			//currentDir = "CW";
		}

		//Serial.print("Direction: ");
		//Serial.print(currentDir);
		//Serial.print(" | Counter: ");
		//Serial.println(counter);
    
    //stopping counterX from having a value that is <0 or <480
    if (counterY <= 0) {
    counterY = 0;
    }
    else if (counterY >= 320) {
      counterY = 320;
    }
	}

  //Remember last CLK state
  lastStateCLK2 = curStateCLK2;

  // tft.setCursor(20, 120);
  // tft.setTextFont(1);  tft.setTextSize(1);
  // tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // tft.print("Rotary Encoder Value:"); tft.print(counterY);
}

//  sketchRot()
//  If integer x is 2 or 0, the funtion will draw with Black or White, Respectively.
//  The function then sketches the Etch-a-sketch (rotary encoder) data.
//
//  Passes an integer x into the function
//
void sketchRot(int x){
  xRotEn();
  yRotEn();
  if (x == 2) {
    tft.drawPixel(counterX, counterY, TFT_BLACK);
  }
  else {
    tft.drawPixel(counterX, counterY, TFT_WHITE);
  }
  
}

//***************************
//  Tilt-A-Sketch Functions
//***************************

//  runTiltA()
//  Utilizes the below functions to run a similar version of Etch-A-Sketch.
//    While button 1 is not held down, Tilt-A-Sketch will run. Once b1 is 
//    held down, the coordinates will be reset, and user is sent to home.
//
//  Returns nothing.
//
void runTiltA() {
  delay(50);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2); tft.setTextColor(WHITE, BLACK);
  
  //TiltA will run until button is held down
  x = 0;
  while (x != 1){
    x = checkB1();
    checkShake();
    updateAngle();
    angleEtch();
  }

  // Resets the starting coordinates
  xVal = 240;
  yVal = 160;

  // Resets b1Bool to FALSE
  b1Bool = false;

  //clears screen and brings back home
  tft.fillScreen(TFT_BLACK);
  displayHome();
}

//  updateAngle()
//  Stores two sets of X and Y angle data into the angArray array.
//    Prints the array into the serial monitor.
//
//  Returns nothing.
//
void updateAngle(){
  angArray[0] = angArray[2];
  angArray[1] = angArray[3];
  mpu6050.update();
  newXA = int(mpu6050.getAccAngleX());
  newYA = int(mpu6050.getAccAngleY());
  angArray[2] = int(newXA);
  angArray[3] = int(newYA);
  //Serial.println(int(mpu6050.getAccAngleX()));

  for (int i=0; i < 4; i++){
    Serial.print(angArray[i]);
    Serial.print(", ");
  }
  Serial.println("");
  // delay(125);
}

//  angleEtch()
//  Determines the angle of the MPU6050 accelerometer. Moves in the 4 cardinal directions
//  as well as 12 more equal angles for a total of 16 directions of movement. Additionally,
//    if the MPU6050 is not tilted in any particular direction, no movement occurs. Prints
//    the direction of movement in the serial monitor after each update.
//
//  The data is taken from an array and placed into variables.
//
//  Returns Nothing.
//
void angleEtch(){
  float angle2 = angArray[2];
  float angle3 = angArray[3];

  // North
  if (tolerances[0] > angle2 && angle2 > tolerances[2] && angle3 < tolerances[1]) {
    Serial.println("NORTH");
    yVal-=2;
  } 
  // South
  else if (tolerances[0] > angle2 && angle2 > tolerances[2] && angle3 > tolerances[3]) {
    Serial.println("SOUTH");
    yVal+=2;
  } 
  // West
  else if (tolerances[1] < angle3 && angle3 < tolerances[3] && angle2 > tolerances[0]) {
    Serial.println("WEST");
    xVal-=2;
  } 
  // East
  else if (tolerances[1] < angle3 && angle3 < tolerances[3] && angle2 < tolerances[2]) {
    Serial.println("EAST");
    xVal+=2;
  } 
  // North East, North-North East, and North East-East
  else if (angle2 < tolerances[2] && angle3 < tolerances[1]) {
    if (abs(angle2 - angle3) <= 5) {
      Serial.println("NORTH EAST");
      yVal-=2;
      xVal+=2;
    }
    else if (angle2 > angle3) {
      Serial.println("NORTH-NORTH EAST");
      yVal-=2;
      xVal+=1;
    }
    else if (angle2 < angle3) {
      Serial.println("NORTH EAST-EAST");
      yVal-=1;
      xVal+=2;
    }
  } 
  // South East, South-South East, and South East-East
  else if (angle2 < tolerances[2] && angle3 > tolerances[3]) {
    if (abs(abs(angle2) - abs(angle3)) <= 5) {
      Serial.println("SOUTH EAST");
      yVal+=2;
      xVal+=2;
    }
    else if (abs(angle2) < abs(angle3)) {
      Serial.println("SOUTH-SOUTH EAST");
      yVal+=2;
      xVal+=1;
    }
    else if (abs(angle2) > abs(angle3)) {
      Serial.println("SOUTH EAST-EAST");
      yVal+=1;
      xVal+=2;
    }
  } 
  // South West, South-South West, and South West-West
  else if (angle2 > tolerances[0] && angle3 > tolerances[3]) {
    if (abs(abs(angle2) - abs(angle3)) <= 5) {
      Serial.println("SOUTH WEST");
      yVal+=2;
      xVal-=2;
    }
    else if (abs(angle2) < abs(angle3)) {
      Serial.println("SOUTH-SOUTH WEST");
      yVal+=2;
      xVal-=1;
    }
    else if (abs(angle2) > abs(angle3)) {
      Serial.println("SOUTH WEST-WEST");
      yVal+=1;
      xVal-=2;
    }
  } 
  // North West, North-North West, and North West-West
  else if (angle2 > tolerances[0] && angle3 < tolerances[1]) {
    if (abs(abs(angle2) - abs(angle3)) <= 5) {
      Serial.println("NORTH WEST");
      yVal-= 2;
      xVal-= 2;
    }
    else if (abs(angle2) < abs(angle3)) {
      Serial.println("NORTH-NORTH WEST");
      yVal-=2;
      xVal-=1;
    }
    else if (abs(angle2) > abs(angle3)) {
      Serial.println("NORTH WEST-WEST");
      yVal-=1;
      xVal-=2;
    }
  } 
  // If not tilted an any particular direction, there will be no movement
  else if (tolerances[0] > angle2 && angle2 > tolerances[2] && tolerances[1] < angle3 && angle3 < tolerances[3]) {
    Serial.println("BALANCED");
  }
  if (xVal < 0) {
    xVal = 0;
  }
  else if (xVal > 480) {
    xVal = 480;
  }
  else if (yVal < 0) {
    yVal = 0;
  }
  else if (yVal > 360) {
    yVal = 360;
  }
  // Choices for brush style
  tft.fillRect(xVal, yVal, 3, 3, TFT_CYAN);
  //tft.drawPixel(xVal, yVal, TFT_RED);
}

//********************
//  Button Functions
//********************

//  checkB1()
//  Checks to see if the button is pressed or held down
//
//  If B1 is held down, output 1
//  If B1 is pressed, b1Bool will switch between TRUE or FALSE
//
//  Returns x
//
int checkB1() {
  if (digitalRead(b1) == HIGH){
    delay(500);
    if (digitalRead(b1) == HIGH) {
      x = 1;
      delay(50);
    }
    else {
      b1Bool = !b1Bool;
      x = b1Bool ? 2 : 0;
    }
  }
  return x;
}

//  checkB2() 
//  Checks to see if the button is pressed or held down
//
//  If B2 is held down, output 1
//  If B2 is pressed, b1Bool will switch between TRUE or FALSE
//
//  Returns x
//
int checkB2() {
  if (digitalRead(b2) == HIGH){
    delay(500);
    if (digitalRead(b2) == HIGH) {
      x = 1;
      delay(50);
    }
    else {
      b2Bool = !b2Bool;
      x = b2Bool ? 2 : 0;
    }
  }
  return x;
}
