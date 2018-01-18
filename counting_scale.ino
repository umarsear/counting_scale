#include <Q2HX711.h>
#include <LiquidCrystal_I2C.h>

const int resolution=1000;
const byte hx711_data_pin = A3;
const byte hx711_clock_pin = A2;
const int  MaxChars=15;

String commandChars="~CWL";
char   strValue[MaxChars+1];
char   serialInputType=' ';
float  itemWeight=0.00;
int    index=0;
long   total=0;
long   average = 0;
long   tareWeight=0;
String LCDLine1,LCDLine2;
String itemLabel;
bool doTare=false;
bool doCount=false;

Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);
LiquidCrystal_I2C lcd(0x3F,16,2);

void updateLCD () {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(LCDLine1);
  Serial.print("A");
  Serial.println(LCDLine1);
  
  lcd.setCursor(0,1);
  lcd.print(LCDLine2);
  Serial.print("B");
  Serial.println(LCDLine2);
}

long readingAverage(int samples=25,long t=0) {
 total=0;
 
 for (int i=0; i<samples;i++) {
    total=total+((hx711.read()/resolution)-t); 
    delay(10);
 }
 return (total / samples);
}

void _doTare() {
  doTare=true;
}

void setTare() {
  doTare=false;
  LCDLine1="TARE ...";
  LCDLine2="";
  updateLCD();
  digitalWrite(13, !digitalRead(13)); // Toggle LED on pin 13
  tareWeight=readingAverage(25,0);
  digitalWrite(13, !digitalRead(13)); // Toggle LED on pin 13
}

void _doCount() {
  doCount=true;
}
void getItemWeight(float c=25.00)
{
  doCount=false;
  LCDLine1="COUNT ...";
  LCDLine2="";
  updateLCD();
  digitalWrite(13, !digitalRead(13)); // Toggle LED on pin 13
  itemWeight=readingAverage(25,tareWeight) / c;
  Serial.print('I');
  Serial.println(int(itemWeight*100));
  digitalWrite(13, !digitalRead(13)); // Toggle LED on pin 13  
}

void setup() {
  Serial.begin(38400);
  Serial.println("Counting Scale - Version 0.9");
  pinMode(13, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(0, _doTare, CHANGE);
  attachInterrupt(1, _doCount, CHANGE);
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight 
  LCDLine1="Initializing ...";
  LCDLine2="";
  updateLCD();
  tareWeight=readingAverage(25,0);
}

void setLabel(char ch) {
  if(index < MaxChars && (isDigit(ch) or isAlpha(ch) or isWhitespace(ch))) {
    strValue[index++] = ch;
  }
  else
  {
    itemLabel=strValue;
    index = 0;
//    strValue[0]=(char)0;
    memset(strValue, 0, sizeof strValue);
    serialInputType=' ';
  }
}

void setItemWeight(char ch) {
  int tempWeight;
  if(index < MaxChars && isDigit(ch)) {
    strValue[index++] = ch;
  }
  else
  {
    tempWeight =atoi(strValue);
    if(tempWeight > 0){
      itemWeight=tempWeight/100.00;
      
  //    lcd.print(itemWeight);
    }
      index = 0;
      memset(strValue, 0, sizeof strValue);
      serialInputType=' ';
  }
}

void readCommand(char ch) {
  serialInputType=' ';
  switch (ch) {
    
    case '2' : // Count 25
      getItemWeight(25);
      break;

    case '5' : // Count 50
      getItemWeight(50);
      break;

    case '7' : // Count 75
      getItemWeight(75);
      break;

    case '1' : // Count 100
      getItemWeight(100);
      break;

    case 'T' : // Tare
      setTare();
      break;
  }
}

void serialEvent()
{
  while(Serial.available()) 
  {
    char ch = Serial.read();
    
    switch (serialInputType) {
      case 'W' : 
        setItemWeight(ch);
        break;
        
      case 'L' :
        setLabel(ch);
        break;
      
      case 'C' : 
        readCommand(ch);        
        break;      
      case 'V' : 
        Serial.println("Counting Scale - Version 0.9");
        break;
      case ' ' : 
        if (commandChars.indexOf(ch)>0) {
          serialInputType=ch;  
        }
        break;      
     }
   }
}
void loop() {
  if (doTare) {
    setTare();
  }

  if (doCount) {
    getItemWeight(25);
  }
  
  average=readingAverage(25,tareWeight);
  Serial.println(average);
  LCDLine1=String(average*0.715)+"g";

  // if item weight has been set, calculate and show quantity
  if (itemWeight>0) {

    // Do not want to show quantity as negative
    if (average>0) {      
      LCDLine2=String(average / itemWeight,0);
    }
    else {
      LCDLine2="0";
    }

    // if there is an associated label, show it. Otherwise show pcs
    if (itemLabel!="") {
      LCDLine2=LCDLine2+" "+itemLabel;
    }
   else {
      LCDLine2=LCDLine2+" Pcs";
    }
  }
  updateLCD();
}
