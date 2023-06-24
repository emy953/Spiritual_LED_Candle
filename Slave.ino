#include <Wire.h> // Libraria pentru I2C

#define HOUR 60000 //numarul de milisecunde intr-un minut - configurabil pentru experimente

#define NR_COL 5 //numarul de coloane din matrice
#define NR_ROW 4 //numarul de linii din matrice

const uint8_t colPins[NR_COL] = {2, 3, 4, 5, 6}; //pinii de control ai coloanelor
const uint8_t rowPins[NR_ROW] = {8, 9, 12, 11}; //pinii de control ai liniilor

int x,y;
int nr=0;
int minmat;
int rec;
int function;
int t_rand;

unsigned long currentMillis;
unsigned long timestamps[NR_ROW][NR_COL];
uint8_t interval[NR_ROW][NR_COL];

void setup() {
  
  // Pornire magistrala I2C cu adresa aleasa
  Wire.begin(9);   
  // Functia atasata pentru evenimentul de primire de date
  Wire.onReceive(receive);
  //Functia atasata pentru evenimentul de cerere de date
  Wire.onRequest(request);
  
  //Initializare pini
  for(int i = 2; i <= 11; i++)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i,LOW);
  }
}

void loop() {

 // for(int i=0;i<NR_ROW;i++)
 // for(int j=0;j<NR_COL;j++)
  {
   // timestamps[i][j]=millis();
  //  interval[i][j]=1;
  }
  matrixcheck(); //functia care actualizeaza matricea de aprindere daca durata de aprindere a unei lumanari a ajuns la final
  matrix(); //functia care tine aprinse lumanarile corespunzatoare
  
}

void receive(int bytes) //functia executata la primirea de date
{
  rec = Wire.read(); //seteaza timpul de aprindere
  function=Wire.read(); //seteaza comportamentul executat la urmatoarea cerere de date
  minmat=matrixmin(); //calculul minimului din matrice
}

void request() //functia executata la cererea de date
{
  if(function==1) //a fost ales semnalul Cerere
  {
    if(nr<NR_COL*NR_ROW) //daca nu mai sunt lumanari disponibile
    {
      Wire.write((byte)127); //raspunde cu raspuns pozitiv
    }
    else
    {
      Wire.write((byte)minmat); //raspunde cu minumul pana la urmatoarea stingere in minute
    }
  }
  else //a fost ales semnalul Aprindere
  {
    Wire.write((byte)1); //raspunde
    do
    {
      x = random(0,NR_ROW);    
      y = random(0,NR_COL);
    }
    while(timestamps[x][y]!=0);
    timestamps[x][y]=millis(); //seteaza lumanarea ca aprinsa in matrice
    interval[x][y]=rec; //seteaza durata de aprindere a lumanarii
    nr++; //numarul de lumanari aprinse creste
  }
}

int matrixmin() //functia care calculeaza timpul minim pana la stingerea urmatoarei lumanari in minute
{
  unsigned long m=60000*60000; //initializare minim
  for(int i=0;i<NR_ROW;i++)
  {
    for(int j=0;j<NR_COL;j++)
    {
      if(m>(timestamps[i][j] + 60*60000*interval[i][j])&&timestamps[i][j]!=0)
        {
          m=timestamps[i][j]+ 60*60000*interval[i][j];
        }
    }
  }
  m=(m-millis())/60000; //transforma din timpul absolut al stingerii (milisecunde) in timpul pana la stingere (minute)
  return m;
}

void matrixcheck() //functia care actualizeaza matricea de aprindere daca durata de aprindere a unei lumanari a ajuns la final
{
  for(int i=0;i<NR_ROW;i++)
  {
    for(int j=0;j<NR_COL;j++)
    {
      currentMillis = millis();
      if((unsigned long)(currentMillis - timestamps[i][j]) >= HOUR*60*interval[i][j] && timestamps[i][j]!=0)
      {
        timestamps[i][j]=0; //seteaza lumanarea ca stinsa in matrice
        interval[x][y]=0;
        nr--;
      }
    }
  }
}

void matrix() //functia care tine aprinse lumanarile corespunzatoare
{
  for(int i=0;i<NR_ROW;i++) //parcurgerea si activarea pe rand a fiecarei linii
  {
    off();
    digitalWrite(rowPins[i],HIGH);
    for(int j=0;j<NR_COL;j++) // parcurgerea si activarea coloanelor corespunzatoare lumanarilor aprinse
    { 
      currentMillis = millis();
      if(timestamps[i][j]!=0)
      {
        if((unsigned long)(currentMillis - timestamps[i][j]) <= 900) //lumanarea se parinde si se stinde intermintent la aprindere
        {
          if((unsigned long)(currentMillis - timestamps[i][j]) % 300 <= 150)
            digitalWrite(colPins[j],HIGH);
          else
            digitalWrite(colPins[j],LOW);
        }
        else
          digitalWrite(colPins[j],HIGH);
      }
      else
      digitalWrite(colPins[j],LOW);
      t_rand=random(5,3000); // alegerea unui delay aleator pentru un efect de flacara
      if(t_rand>=2998)
      delayMicroseconds(t_rand/4); 
    }
  delayMicroseconds(10);
  }
}

void off() //functia de stingere a tuturor lumanarilor
{

for(int i=0;i<NR_ROW;i++)
  digitalWrite(rowPins[i], LOW);  
for(int i=0;i<NR_COL;i++)
  digitalWrite(colPins[i], LOW);
  
}
