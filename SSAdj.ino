#include <EEPROM.h>



#define DEBUG1 0
#define DEBUG2 0
#define DEBUGFUNC 0
#define DEBUGDATA 0

int analogPin1 = A0;

int val1 = 0;
int curState1 = 0;

float earOnPlaceThr1 = 362;
float earOffPlaceThr1 = 356;

//int offset1 = (earOnPlaceThr1 - earOffPlaceThr1);
float k1 = 0.3;
float offsetOff1 = 1;//float(offset1) * (k1);

int analogPin2 = A1;

int val2 = 0;
int curState2 = 0;

float earOnPlaceThr2 = 362;
float earOffPlaceThr2 = 356;

//int offset2 = (earOnPlaceThr2 - earOffPlaceThr2);
float k2 = 0.3;
float offsetOff2 = 1;//float(offset2) * (k2);

bool adjustOn = false;
bool adjustOff = false;

const float averageN = 300;
float averageI = 0;
float xoxo;

const char *err = "error";
bool init(float* val, int addr) {
  xoxo = 0;
  EEPROM.get(addr, xoxo);
  if (xoxo != 0.0) {
    *val = xoxo;
    return true;
  }
  else
    return false;
}

void setup()
{
 
  Serial.begin(9600);
  while (!Serial) {}

 
  int eeAddress = 0;
  if (!init(&earOnPlaceThr1, eeAddress) && DEBUGFUNC)
    Serial.println(err);

  eeAddress += sizeof(float);
  if (!init(&earOffPlaceThr1, eeAddress) && DEBUGFUNC)
    Serial.println(err);

  eeAddress += sizeof(float);
  if (!init(&earOnPlaceThr2, eeAddress) && DEBUGFUNC)
    Serial.println(err);

  eeAddress += sizeof(float);
  if (!init(&earOffPlaceThr2, eeAddress) && DEBUGFUNC)
    Serial.println(err);

  Serial.begin(9600);          //  setup serial
  analogReference(INTERNAL);
  pinMode(13, OUTPUT);
}

void loop()
{
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if (DEBUGFUNC) {
      Serial.print("recv ");
      Serial.println(incomingByte);
    }

    if (incomingByte == 'd') {
      averageI = 0;
      adjustOn = true;
      adjustOff = false;
      earOnPlaceThr1 = 0;
      earOnPlaceThr2 = 0;
    }

    if (incomingByte == 'e') {
      averageI = 0;
      adjustOn = false;
      adjustOff = true;
      earOffPlaceThr1 = 0;
      earOffPlaceThr2 = 0;
    }
  }
  
  if (!adjustOn && !adjustOff) {
    normal1();
    normal2();
  }
  
  if (adjustOn)
    adjustOnF();
  if (adjustOff)
    adjustOffF();
}


void normal1() {
  val1 = analogRead(analogPin1);

  float diff1 = (earOnPlaceThr1 - earOffPlaceThr1);
  float diff1upper = diff1*k1*0.5;
  float diff1lower = diff1*k1;

  if (val1 >= earOnPlaceThr1 - diff1upper  && curState1 == 0) {
    Serial.println("1");
    digitalWrite(13, HIGH);
    curState1 = 1;
  }
  if (val1 < earOffPlaceThr1 + diff1lower && curState1 == 1) {
    Serial.println("0");
    digitalWrite(13, LOW);
    curState1 = 0;
  }
  if (DEBUG1)
    Serial.println(val1);
}

void normal2() {
  val2 = analogRead(analogPin2);

  float diff2 = (earOnPlaceThr2 - earOffPlaceThr2);
  float diff2upper = diff2*k2*0.5;
  float diff2lower = diff2*k2;
  
  if (val2 >= earOnPlaceThr2 - diff2upper && curState2 == 0) {
    Serial.println("3");
    digitalWrite(13, HIGH);
    curState2 = 1;
  }
  if (val2 < earOffPlaceThr2 + diff2lower && curState2 == 1) {
    Serial.println("2");
    digitalWrite(13, LOW);
    curState2 = 0;
  }
  if (DEBUG2)
    Serial.println(val2);
}

void adjustOnF() {
  averageI += 1;
  if (averageI > averageN) {
    adjustOn = false;
    if (DEBUGFUNC) {
      Serial.println("Received");
      Serial.println("earOnPlaceThr1");
      Serial.println(earOnPlaceThr1);
      Serial.println("earOnPlaceThr2");
      Serial.println(earOnPlaceThr2);
    }
    return;
  }
  float fval1 = analogRead(analogPin1);
  float fval2 = analogRead(analogPin2);

  if (DEBUGDATA) {
    fval1 = 362 + random(5);
    fval2 = 362 + random(5);
  }

  earOnPlaceThr1 += fval1 / averageN;
  earOnPlaceThr2 += fval2 / averageN;

  delay(50);
}


void adjustOffF() {
  averageI += 1;
  if (averageI > averageN) {
    adjustOff = false;
    if (DEBUGFUNC) {
      Serial.println("Received");
      Serial.println("earOffPlaceThr1");
      Serial.println(earOffPlaceThr1);
      Serial.println("earOffPlaceThr2");
      Serial.println(earOffPlaceThr2);
    }

    if (earOnPlaceThr1 > earOffPlaceThr1 ||
        earOnPlaceThr2 > earOffPlaceThr2) {
      int eeAddress = 0;

      EEPROM.put(eeAddress, earOnPlaceThr1);
      eeAddress += sizeof(float);
      EEPROM.put(eeAddress, earOffPlaceThr1);
      eeAddress += sizeof(float);
      EEPROM.put(eeAddress, earOnPlaceThr2);
      eeAddress += sizeof(float);
      EEPROM.put(eeAddress, earOffPlaceThr2);
      Serial.write('s');
    }
    else
      Serial.write('e');
    return;
  }
  float fval1 = analogRead(analogPin1);
  float fval2 = analogRead(analogPin2);
  if (DEBUGDATA) {
    fval1 = 356 + random(5);
    fval2 = 356 + random(5);
  }

  earOffPlaceThr1 += fval1 / averageN;
  earOffPlaceThr2 += fval2 / averageN;

  delay(50);
}


