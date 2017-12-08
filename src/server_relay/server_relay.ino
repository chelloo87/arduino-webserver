/*
 * Rui Santos 
 * Complete Project Details http://randomnerdtutorials.com
*/

#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"

#include "floatToString.h"


char testH[20];
char testT[20];

#define DHTPIN 6     
#define DHTTYPE DHT22 //DHT11, DHT21, DHT22

DHT dht(DHTPIN, DHTTYPE);

const float maxTemp = 22;
const float minTemp = 21.5;

bool isTempAuto = false;

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,0,121);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

// Relay state and pin
String relay1State = "Off";
String relay2State = "Off";
const int relay = 7;
const int relay2 = 8;
const int DTH = 6;

// Client variables 
char linebuf[80];
int charcount=0;

void setup() { 
  // Relay module prepared 
  pinMode(relay, OUTPUT);
  pinMode(relay2, OUTPUT);
  digitalWrite(relay, HIGH);
    digitalWrite(relay2, HIGH);
  
  // Open serial communication at a baud rate of 9600
  Serial.begin(9600);
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
//   if (Ethernet.begin(mac) == 0)  // Start in DHCP Mode
//  {
//    Serial.println("Failed to configure Ethernet using DHCP, using Static Mode");
//    // If DHCP Mode failed, start in Static Mode
//    Ethernet.begin(mac, ip);
//  }
  
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
    dht.begin(); 
}

// Display dashboard page with on/off button for relay
// It also print Temperature in C and F
void dashboardPage(EthernetClient &client) {
  client.println("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
  client.println("<!DOCTYPE HTML><html><head>");
  client.println("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"></head><body>");                                                             
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><body>");                                                             
  client.println("<h3>Arduino Web Server - <a href=\"/\">Refresh</a></h3>");
  // Generates buttons to control the relay
  client.println("<h4>Pompa caldura - State: " + relay1State + "</h4>");
  // If relay is off, it shows the button to turn the output on          
  if(relay1State == "Off"){
    client.println("<a href=\"/relay1on\"><button>ON</button></a>");
  }
  // If relay is on, it shows the button to turn the output off         
  else if(relay1State == "On"){
    client.println("<a href=\"/relay1off\"><button>OFF</button></a>");                                                                    
  }
  
  client.println("<h4> Limina casa - State: " + relay2State + "</h4>");
  // If relay is off, it shows the button to turn the output on          
  if(relay2State == "Off"){
    client.println("<a href=\"/relay2on\"><button>ON</button></a>");
  }
  // If relay is on, it shows the button to turn the output off         
  else if(relay2State == "On"){
    client.println("<a href=\"/relay2off\"><button>OFF</button></a>");                                                                    
  }
  
  if(isTempAuto == true){
    client.println("<a style='color:green' href=\"/relay1auto\"><button>AUTO</button></a>");
  }
  else{
     client.println("<a style='color:red' href=\"/relay1auto\"><button>AUTO</button></a>");   
  }
  

  float h = dht.readHumidity();    
  float t = dht.readTemperature();  

  floatToString(testH, h, 2, 6);
  floatToString(testT, t, 2, 6);

Serial.println(h);
Serial.println(t);
  client.println("<h4>Temp  ");   client.print(testT); client.print(" C</h4>");
  client.println("<h4>Hum  ");   client.print(testH); client.print(" %</h4>");
  client.println("<h4>Min temp");   client.print(minTemp); client.print(" C</h4>");
  client.println("<h4>Max temp");   client.print(maxTemp); client.print(" C</h4>");
  client.println("</body></html>"); 
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    memset(linebuf,0,sizeof(linebuf));
    charcount=0;
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
       char c = client.read();
       //read char by char HTTP request
        linebuf[charcount]=c;
        if (charcount<sizeof(linebuf)-1) charcount++;
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          dashboardPage(client);
          break;
        }
        if (c == '\n') {
          if (strstr(linebuf,"GET /relay1off") > 0){
            digitalWrite(relay, HIGH);
            relay1State = "Off";
          }
          else if (strstr(linebuf,"GET /relay1on") > 0){
            digitalWrite(relay, LOW);
            relay1State = "On";
          }
          
          if (strstr(linebuf,"GET /relay2off") > 0){
            digitalWrite(relay2, HIGH);
            relay2State = "Off";
          }
          else if (strstr(linebuf,"GET /relay2on") > 0){
            digitalWrite(relay2, LOW);
            relay2State = "On";
          }
          
          else if (strstr(linebuf,"GET /relay2auto") > 0){
            digitalWrite(relay2, LOW);
            relay2State = "On";
            isTempAuto = true;
          }
          // you're starting a new line
          currentLineIsBlank = true;
          memset(linebuf,0,sizeof(linebuf));
          charcount=0;          
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1000);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
    
    float h = dht.readHumidity();    
  float t = dht.readTemperature();
    
    if(t+0.4>= maxTemp){
      digitalWrite(relay, HIGH);
      Serial.println("<");
      Serial.println(t);
      relay1State = "Off";
      
      
    }
    
    
    if(t<=minTemp){
      digitalWrite(relay, LOW);
      relay1State = "On";
      Serial.println(">");
      Serial.println(t);
      
    }
    
    
  }
}
