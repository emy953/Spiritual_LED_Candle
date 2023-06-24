#include <Adafruit_GFX.h>    // Libraria grafica de baza
#include <Adafruit_ST7735.h> // Libraria pentru ST7735

#include <SPI.h> //libraria SPI

#include<Wire.h> //libraria I2C

#define TFT_CS         10 //ST7735 Chip Select
#define TFT_RST        8  //ST7735 Reset
#define TFT_DC         9  //ST7735 Data/Control

#define TFT_SCLK 13 //ST7735 Clock
#define TFT_MOSI 11 //ST7735 SDA

#define WELCOME 1 //Starea de asteptare de bun venit
#define CONFIRM 2 //Starea de asteptare a confirmarii

//Initializare display TFT 1.44"
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

int valid[128];
int mark[128];
int n_valid;
int n_mark;

int signals;

int state;

int slave;

int time_min=300;
int timp;

int buton1=2;
int buton2=3;
int signal_in=6;
int supress=5;
int k;

int ant_value1,ant_value2,value1,value2;

unsigned long press_time;
int interval=60000;

void setup(void) {
  //Initializare maginstrala I2C cu rolul de master
  Wire.begin();
  Serial.begin(9600);
  //Alegere timp de asteptare pana la declararea adresei ca invalida
  Wire.setWireTimeout(1000);

  //Initializare pini ST7735
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(5,OUTPUT);

  //Initializare pini pentru ICT P85
  digitalWrite(supress,HIGH);
  pinMode(6,INPUT);
  
  tft.initR(INITR_144GREENTAB); //Initializare ST7735R, green tab

  tft.fillScreen(ST77XX_BLACK); //Setare ecran display negru

  SetupMessage(); //Mesajul de start

  detectSlaves(); //functia de testare a adreselor

  if(n_valid==0)
  {
    ErrorMessage(); //Mesajul de eroare
    while(1){}
  }
  else
  {
    OkMessage(); //Mesajul de sistem functional
    WelcomeMessage(); //Mesajul de bun venit si intrarea in starea de asteptare
    ant_value1=ant_value2=value1=value2=1; //Initializare valori pentru citirea butoanelor
  }
}

void loop() {
  //Citirea butoanelor
  value1=digitalRead(buton1);
  value2=digitalRead(buton2);
  if(ant_value1==LOW && value1==HIGH)
  {
    press_buton(1);
  }
  if(ant_value2==LOW && value2==HIGH)
  {
    press_buton(2);
    k=1;
  }
  ant_value1=value1;
  ant_value2=value2;

  if(state==CONFIRM) //Activarea si citirea acceptorului de bancnote in starea de confirmare
  {
    if(digitalRead(signal_in)==LOW)
    signals++;
    if(signals>5&&signals<20&&timp==2&&k)
    {
      OneMoreMessage();
      k=0;
    }
    if(signals>5&&timp==1||signals>20&&timp==2)
    {
      if(validate(slave))
      {
        int result;
        Wire.beginTransmission(slave);
        Wire.write((byte)timp);    
        Wire.write((byte)2);    
        Wire.endTransmission();  
        Wire.requestFrom(slave,1);
        while (Wire.available()) {
        result= Wire.read(); 
        }
        if(result==1)
        {
          PositiveMessage();
          WelcomeMessage();
        }
      }
      else
      {
        if(k)
        {
          ErrorMessage();
          while(1){}
        }
      }
    }
    if((unsigned long)(millis()-press_time)>=interval&&signals==0)
    {
      CancellMessage();
      WelcomeMessage();
      state=WELCOME;
    }
  }
}

void press_buton(int tim) //Actiunea la apasarea butonului
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);

  if(state==WELCOME) //actiunea de alegere a duratei de aprindere
  {
    if(requestLED(tim))
    {
      if(tim==1)
      tft.print("Ati ales 1 ora");
      else
      {
        tft.print("Ati ales ");
        tft.print(tim);
        tft.print(" ore");
      }
      delay(1000);
      signals=0;
      ConfirmMessage();
      press_time=millis();
    }
    else
    {
      NoLEDMessage(time_min);
      delay(1000*3);
      WelcomeMessage();
    }
  }
  else if(state==CONFIRM) //actiunea de anulare a alegerii facute
  {
    CancellMessage();
    WelcomeMessage();
  }
}

int requestLED(int tim) //functia de gasire a unui modul valid cu lumanari disponibile
{
  timp=tim;
  int t;
  for(int i=0;i<128;i++) mark[i]=0; //resetare vector mark
  n_mark=0;
  
  while(n_mark!=n_valid) //gasire modul
  {
    do
    {
      slave = random(0,128);
    }
    while(valid[slave]==0||mark[slave]==1);
    byte result,interval;
    int k=0;
    Wire.beginTransmission(slave);
    Wire.write((byte)timp);    
    Wire.write((byte)1);    
    Wire.endTransmission();  
    k=0;
    Wire.requestFrom(slave,1);
    while (Wire.available()) {
    result = Wire.read();
    }
    Serial.println(result);
    if(result!=127)
    {
      mark[slave]=1;
      n_mark++;
      if(time_min>result)
      time_min=result;
    }
    else
    {
      return 1;
    }
  }
    
  if(n_valid==0) //daca nu mai sunt module valide => eroare
  {
    ErrorMessage();
    while(1){}
  }
  
  return 0; // exista module valide dar niciunul nu are lumanari disponibile
}

void detectSlaves() //functia de testarea a adreselor
{
  for(int i=0;i<128;i++)
  {
    Serial.print("Detect ");
    Serial.println(i);
    valid[i]=0;
    if(validate(i)) // daca e valid
    {
      valid[i]=1;
      n_valid++;
    }
  }
}

int validate(int adr) //functia de validare a adresei
{
  
  Wire.beginTransmission(adr);
  byte error = Wire.endTransmission();
  if(error!=0)
  return 0;
  
  return 1;
}

void OneMoreMessage() //Mesajul de continuare a introducerii bancnotelor
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Va rugam mai");
  tft.setCursor(0, 10);
  tft.print("introduceti 1 leu");
  tft.setCursor(0, 30);
}

void NoLEDMessage(int mi) //Mesajul afisat cand nu mai sunt lumanari disponibile in sistem
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Nu sunt lumanari");
  tft.setCursor(0, 10);
  tft.print("disponibile");
  tft.setCursor(0, 30);
  tft.print("Va rugam asteptati");
  tft.setCursor(0, 50);
  tft.print("Urmatoarea lumanare");
  tft.setCursor(0, 60);
  if(mi==0)
  {
    tft.print("se va stinge in mai");
    tft.setCursor(0, 70);
    tft.print("putin de un minut");
  }
  else if(mi==1)
  {
    tft.print("se va stinge intr-un");
    tft.setCursor(0, 70);
    tft.print("minut");
  }
  else
  {
    tft.print("se va stinge in ");
    tft.setCursor(0, 70);
    tft.print(mi);
    tft.print(" minute");
  }
  delay(1000);
}

void PositiveMessage() //Mesajul de aprindere a lumanarii
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("O lumanare a fost");
  tft.setCursor(0, 10);
  tft.print("aprinsa");
  delay(1000);
}

void CancellMessage() //Mesajul de anulare a alegerii
{
  digitalWrite(supress,HIGH);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Alegerea a fost");
  tft.setCursor(0, 10);
  tft.print("anulata");
  delay(1000);
}
      
void ConfirmMessage() //Mesajul de cerere a confirmarii alegerii
{
  digitalWrite(supress,LOW);
  tft.setCursor(0, 20);
  tft.print("Va rugam confirmati");
  tft.setCursor(0, 30);
  tft.print("alegerea facuta");
  tft.setCursor(0, 50);
  if(timp==1)
  tft.print("Inserati un leu");
  else
  tft.print("Inserati doi lei");
  tft.setCursor(0, 60);
  tft.print("pentru confirmare");
  tft.setCursor(0, 80);
  tft.print("Apasati oricare");
  tft.setCursor(0, 90);
  tft.print("buton pentru anulare");
  state=CONFIRM;
}

void OkMessage() //Mesajul de pornire a sistemului functional
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Sistemul este pornit!");
  tft.setCursor(0, 10);
  if(n_valid>1)
  {
    tft.print("Functioneaza ");
    tft.print(n_valid);
    tft.print(" module");
  }
  else
  {
        tft.print("Functioneaza un modul");
  }
  delay(3000);
}

void ErrorMessage() //Mesajul de eroare
{
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Sistemul nu functioneaza");
  tft.setCursor(0, 10);
  tft.print("Va rugam contactati asistenta tehinca");
  
}

void SetupMessage() //Mesajul initial de pornire
{
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Pornire");
}

void WelcomeMessage() //Mesajul de bun venit
{
  digitalWrite(supress,HIGH);
  state=WELCOME;
  //testfillrects(ST77XX_YELLOW, ST77XX_MAGENTA);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_BLUE,ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextWrap(true);
  tft.print("Bine ati venit");
  tft.setCursor(0, 10);
  tft.print("Va rugam alegeti");
  tft.setCursor(0, 20);
  tft.print("timpul de aprindere");
  tft.setCursor(0, 55);
  tft.print("1 ora - 1 leu");
  tft.setCursor(0, 90);
  tft.print("2 ore - 2 lei");
}
