#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <time.h>
#include "DHT.h"

///////////////////////////////////////////////////
#include <ESP8266WebServer.h>
String inputMessage = "Have a nice day!";
//////////////////////////////////////////////////

int pinCS = 2; 
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays   = 1;
char time_value[20];

// LED Matrix Pin -> ESP8266 Pin
// Vcc            -> 3v  (3V on NodeMCU 3V3 on WEMOS)
// Gnd            -> Gnd (G on NodeMCU)
// DIN            -> D7  (Same Pin for WEMOS)
// CS             -> D4  (Same Pin for WEMOS)
// CLK            -> D5  (Same Pin for WEMOS)

Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 70; // In milliseconds

int spacer = 1;
int width  = 5 + spacer; // The font width is 5 pixels

int m;

#define DHTPIN 0          // D3

#define DHTTYPE DHT11     // DHT 11

DHT dht(DHTPIN, DHTTYPE);

/////////////////////////////////////////////////
ESP8266WebServer server ( 80 );
char htmlResponse[3000];

void handleRoot() {

  snprintf ( htmlResponse, 3000,
"<!DOCTYPE html>\
<html lang=\"en\">\
  <head>\
    <meta charset=\"utf-8\">\
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
  </head>\
  <body>\
          <h1>Ca nha ta cung yeu thuong nhau</h1>\
          <h1>Input message</h1>\
          <input type='text' name='input_message' id='input_message' size=10 autofocus> \
          <div>\
          <br><button id=\"save_button\">Display</button>\
          </div>\
    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js\"></script>\    
    <script>\
      var hh;\
      $('#save_button').click(function(e){\
        e.preventDefault();\
        hh = $('#input_message').val();\       
        $.get('/save?hh=' + hh, function(data){\
          console.log(data);\
        });\
      });\      
    </script>\
  </body>\
</html>"); 

   server.send ( 200, "text/html", htmlResponse );  

}



void handleSave() {
  if (server.arg("hh")!= ""){
    inputMessage = server.arg("hh");
  }
} 

/////////////////////////////////////////////////

String t, h;

void setup() {
  Serial.begin(9600);
  delay(10);
  dht.begin();
 
  //INSERT YOUR SSID AND PASSWORD HERE
  WiFi.begin("Home","244466666");

  //CHANGE THE POOL WITH YOUR CITY. SEARCH AT https://www.ntppool.org/zone/@
  
  configTime(7 * 3600, 0, "0.vn.pool.ntp.org", "time.nist.gov");
  
  /////////////////////
  server.on ( "/", handleRoot );
  server.on ("/save", handleSave);

  server.begin();
  /////////////////////
  
  matrix.setIntensity(0); // Use a value between 0 and 15 for brightness
  matrix.setRotation(0, 1);    // The first display is position upside down
  matrix.setRotation(1, 1);    // The first display is position upside down
  matrix.setRotation(2, 1);    // The first display is position upside down
  matrix.setRotation(3, 1);    // The first display is position upside down
  matrix.fillScreen(LOW);
  matrix.write();
  
  while ( WiFi.status() != WL_CONNECTED ) {
    matrix.drawChar(2,0, 'W', HIGH,LOW,1); // H
    matrix.drawChar(8,0, 'I', HIGH,LOW,1); // HH  
    matrix.drawChar(14,0,'-', HIGH,LOW,1); // HH:
    matrix.drawChar(20,0,'F', HIGH,LOW,1); // HH:M
    matrix.drawChar(26,0,'I', HIGH,LOW,1); // HH:MM
    matrix.write(); // Send bitmap to display
    delay(250);
    matrix.fillScreen(LOW);
    matrix.write();
    delay(250);
  }
  display_message(WiFi.localIP().toString());
  delay(1000);
}

void loop() {
  server.handleClient();
  
  m = map(analogRead(0),0,1024,0,12);
  matrix.setIntensity(m);
  matrix.fillScreen(LOW);
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  //Serial.println(time);
  time.substring(11,19).toCharArray(time_value, 10); 
  matrix.drawChar(2,0, time_value[0], HIGH,LOW,1); // H
  matrix.drawChar(8,0, time_value[1], HIGH,LOW,1); // HH  
  matrix.drawChar(14,0,time_value[2], HIGH,LOW,1); // HH:
  matrix.drawChar(20,0,time_value[3], HIGH,LOW,1); // HH:M
  matrix.drawChar(26,0,time_value[4], HIGH,LOW,1); // HH:MM
  matrix.write(); // Send bitmap to display

  int i = 100;
  while(--i){
    server.handleClient();
    delay(100);
  }
      
  matrix.fillScreen(LOW);
  h = (String)(int)dht.readHumidity();
  t = (String)(int)dht.readTemperature();
  display_message(t+"C "+h+"%");
  delay(100);
  display_message(inputMessage);
//  Serial.println(inputMessage);
}

void display_message(String message){
   for ( int i = 0 ; i < width * message.length() + matrix.width() - spacer; i++ ) {
    //matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < message.length() ) {
        matrix.drawChar(x, y, message[letter], HIGH, LOW, 1); // HIGH LOW means foreground ON, background off, reverse to invert the image
      }
      letter--;
      x -= width;
    }
    matrix.write(); // Send bitmap to display
    delay(wait/2);
  }
}
