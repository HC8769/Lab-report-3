#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Wire.h>
#include <MPU6050_tockn.h>

#define I2C_SLAVE_ADDR 0x04

bool beginNav = false;
int inp[50] = {0}, type = 1, x = 0, newDist, newAng, leftSpeed, rightSpeed, steeringAngle, num = 0, angleZ;
char dataString[5], key;
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
 {'1','2','3'},
 {'4','5','6'},
 {'7','8','9'},
 {'*','0','#'}
};

byte rowPins[ROWS] = {12, 14, 27, 26}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {25, 33, 32}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(19, 23, 17, 16, 13, 2);
MPU6050 mpu6050(Wire);

void setup() {
  Serial.begin(9600);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("input distance");
  lcd.setCursor(0,1);

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  stop();

  inp[50] = {0};
  bool beginNav = false;
  num = keypad.getKey();
  x = 0;
  type = 1;
}
 
void loop() {
  getAngleZ();

  //take input from keypad
  while (beginNav == false)
  {
    key=keypad.getKey();
    if(key)
    {
      inputData();
    }
  }

  deriveData();
}

void inputData()
{
  if(key == '#')
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Begin Navigation");
    Serial.println("Begin Navigation");
    inp[x + 1] = -2;
    x = x + 2;
    beginNav = true;
    type = 1;
  }
  else if(key == '*')
  {
    lcd.setCursor(0,0);
    switch(type)
    {
      case -1:
        lcd.clear();
        lcd.print("input distance");
        Serial.print("input distance");
        type = 1;
      break;

      case 1:
        Serial.println("distance is " + inp[x]);
        lcd.clear();
        lcd.print("input angle");
        Serial.print("input angle");
        type = -1;
      break;
    }
    Serial.println(type);
    lcd.setCursor(0,1);
    inp[x + 1] = -1;
    x = x + 2;
  }  
  else
  {
    if(key)
    {
    lcd.print(key);
    }
   
    switch(type)
    {
      case -1:
        inp[x] = key - '0';
      break;

      case 1:
        inp[x] = (inp[x] * 10) + (key - '0');
      break;
    }
  }
  key = ' ';
  Serial.println(inp[x]);
}

void deriveData()
{
  int current = inp[num];

  switch(current)
  {
    case -2:
      stop();
      delay(1000);
      setup();
    break;
     
    case -1:
      type = type * -1;
    break;

    default:
      if(type = 1)
      {
        long newPosition = myEnc.read();
        int encodeDistance = current * 50;
        Serial.println(current * 10);
        turnStraight();
        while (newPosition != encodeDistance);{
          turnStraight();
      }
      else if(type = -1)
      {
        if(current = 1)
        {
          Serial.println("Turn Left 90 Degrees");
          turnLeft();
          getAngleZ();
          int nextAng = angleZ + 80;
          while(angleZ < nextAng)
          {
            getAngleZ();
          }
          turnStraight();
          stop();        
        }    
        else if(current = 3)
        {
          Serial.println("Turn Right 90 Degrees");
          turnRight();
          getAngleZ();
          int nextAng = angleZ - 80;
          while(angleZ > nextAng)
          {
            getAngleZ();
          }
            turnStraight();
            stop();
        }  
      }
    break;
  }

  Serial.println("End:");
  Serial.print("num: \t");
  Serial.print(num);
  Serial.print("\tcurrent: ");
  Serial.print(current);
  Serial.print("\ttype: ");
  Serial.println(type);

  num++;
  stop();
  delay(400);
}

void getAngleZ() {
  mpu6050.update(); //get new position of gyroscope
  angleZ = int(mpu6050.getAngleZ()); //send integer of angle
  Serial.print("angleZ: ");
  Serial.println(int(mpu6050.getAngleZ()));
}

void sendData(int x, int y, int z)
{ Wire.beginTransmission(I2C_SLAVE_ADDR);
  Wire.write((byte)((x & 0x0000FF00) >> 8));
  Wire.write((byte)(x & 0x000000FF));
  Wire.write((byte)((y & 0x0000FF00) >> 8));
  Wire.write((byte)(y & 0x000000FF));
  Wire.write((byte)((z & 0x0000FF00) >> 8));
  Wire.write((byte)(z & 0x000000FF));
  Wire.endTransmission();
}

void stop() {
  //stop
  leftSpeed = 0;
  rightSpeed = 0;
  steeringAngle = 58;
  sendData(leftSpeed, rightSpeed, steeringAngle);
}

void turnLeft() {
  //turn left
  steeringAngle = 20;
  leftSpeed = 100;
  rightSpeed = -255;
  sendData(leftSpeed, rightSpeed, steeringAngle);
}

void turnRight() {
  //turn right
  steeringAngle = 85;
  leftSpeed = 255;
  rightSpeed = -100;
  sendData(leftSpeed, rightSpeed, steeringAngle);
}

void turnStraight() {
  //turn right
  steeringAngle = 58;
  leftSpeed = 255;
  rightSpeed = -255;
  sendData(leftSpeed, rightSpeed, steeringAngle);
}
