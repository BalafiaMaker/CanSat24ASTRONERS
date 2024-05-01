 #include <SoftwareSerial.h>
#include <TinyGPS.h>
#include "DFRobot_EnvironmentalSensor.h" //LIBRERIA MULTISENSOR
#include <SPI.h>
#include <SD.h>
//Neopixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

//Neo pixel
#define LED_PIN    5 //pin Arduino que está conectada la tira de Pixels.
#define LED_COUNT 8 // Número de pixels en la tira
// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
 int colorR = 0; // Color del led de la tira de neopixel
  int colorG = 0; // Color del led de la tira de neopixel
  int colorB = 127; // Color del led de la tira de neopixel
// Tarjeta
const int chipSelect = 9;
//GPS
TinyGPS gps;
SoftwareSerial ss(4, 3);

//Multisensor
DFRobot_EnvironmentalSensor environment(0x22, &Wire);
// Variables GPS
static void smartdelay(unsigned long ms);
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);

// Variable paquete
int paquete = 0;

void setup()
{
  pinMode(6, OUTPUT);//Zumbador
////////////////////TIRA NEOPIXEL///////////////////////////////////
 // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10); // Set BRIGHTNESS to about 1/5 (max = 255)
 
//////////////////////////COMUNICACIONES  APC220//////////////////
  Serial.begin(9600); //Puerto serie. Envio de datos APC220
  ss.begin(9600); // Activa el RX/TX del GPS 

  ///////////////Espera a obtener datos del multisensor
  while(environment.begin() != 0){
    Serial.println(" No encuentra sensor, mira los cables!!");
    delay(1000);
  }
  Serial.println(" Sensor ok");
////////////////////Tarjeta
 Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("inicializando SD");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Fallo tarjeta");
    // don't do anything more:
    while (1);

}
 File dataFile = SD.open("CANSAT.txt", FILE_WRITE);

  // Escribir en el archivo de texto
  if (dataFile) {
    dataFile.println();
    dataFile.close();
     delay(10);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening CANSAT.txt");
  }

}

void loop()
{
  // Variables GPS
 float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
 
  // Variables de Multisensor
 float temperatura, humedad, UV, Lumi, Presion, Altitud; 
 int year,UV_index;
 byte month, day, hour, minute, second; 

temperatura = environment.getTemperature(TEMP_C);
humedad = environment.getHumidity();
UV = environment.getUltravioletIntensity();
Lumi = environment.getLuminousIntensity();
Presion = environment.getAtmospherePressure(HPA);
Altitud = environment.getElevation();
UV_index = UV * 4;

 //Multisensor
String dataString = String(paquete++) + "," + String(temperatura) + ","+ String(humedad) + "," + String(UV) +  "," + String(Lumi)+ "," + String(Presion) +="," + String(Altitud) + ",";

// Primer numero de la fila de datos


 gps.f_get_position(&flat, &flon, &age);
gps.crack_datetime(&year, &month, &day, &hour, &minute, &second);
 // Datos GPS
dataString +=  String(flat, 6) + ","+ String(flon, 6) + ",";
 dataString += String(hour) + ":" + String(minute) + ":" + String(second) + ","; // Hora
  dataString += String(day) + "/" + String(month) + "/" + String(year);// Fecha
  
//Abrir archivo de texto CANSAT.txt,
  File dataFile = SD.open("CANSAT.txt", FILE_WRITE);

  // Escribir en el archivo de texto
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    //mandar al puerto serie también:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening CANSAT.txt");
  }
////////SEMÁFORO UVI/////
if(UV_index < 3) {
colorR = 0;colorG = 127;colorB =0;//verde
} 
else if(UV_index < 6){ 
colorR = 127 ;colorG = 127;colorB =0;//amarillo
}
else if(UV_index < 8){ 
colorR = 127 ;colorG = 70;colorB =0;//naranja
}else if(UV_index < 10){ 
colorR = 127 ;colorG = 0;colorB =0;//rojo
}
else if(UV_index <=15){ 
colorR = 0 ;colorG = 0;colorB = 127;//naranja
}
  //digitalWrite(6,HIGH);//suena el zumbador cuando acaba de grabar en la tarjeta.
  theaterChase(strip.Color(colorR,colorG,colorB), 125); // Blue, half brightness
 
  //smartdelay(500);
  digitalWrite(6, LOW);

}
///////////////Función del GPS
static void smartdelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}
/////Funciones de la tira de NEOPIXEL
void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Color(r,g,b) as mentioned above), and a delay time (in ms)
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<1; a++) {  // Repeat 1 times...
    for(int b=0; b<8; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 8...
      for(int c=b; c<strip.numPixels(); c += 8) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      smartdelay(wait);  // Pause for a moment
    }
  }
}


