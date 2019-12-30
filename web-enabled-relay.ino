/*
  Web Server

 A simple web server that parses a request to set the
 state of a relay using an Arduino Wiznet Ethernet. Tested
 with an original Arduino Ethernet board using the original
 Ethernet library. 

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 modified 02 Sept 2015
 by Arturo Guadalupi
 modified 27 Dec 2019
 by Jerry Webb

 */

#include <SPI.h>
#include <Ethernet.h>
#define PIN_LED 2
#define PIN_RELAY_0 A5
#define PIN_RELAY_1 A4
// #define DEBUG 1

// Set a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 32);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  // Initialise interface to relay board
  pinMode(PIN_RELAY_0, OUTPUT);
  pinMode(PIN_RELAY_1, OUTPUT);
  SW(0, LOW);
  SW(1, LOW);
#ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
#ifdef DEBUG
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
#endif
  // Tom's code has checks to make sure the Ethernet board is available and
  // that the drop cable is connected which are removed from this version
  // because there's no way to communicate these errors to the user with the
  // hardware config we're running on.
}


void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
#ifdef DEBUG
    Serial.println("new client");
#endif
    boolean currentLineIsZeroLength = true;
    String line = "";    // Used to assemble a request string from the client if they provide one
    bool firstLine = true;
    int httpResponse = 400;
    
    // an http request ends with an empty line (just a newline)
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
#ifdef DEBUG
        Serial.write(c);
#endif        
        
        if (c == '\n' && currentLineIsZeroLength) {
          // if we have reached the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so we send a reply describing the state of both switches on
          // completion of the request
          // send a standard http response header
          if(httpResponse == 400) {
            client.println("HTTP/1.1 400 Bad Request");
            client.println("Connection: close");
            client.println();
          } 
          else {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: application/json");
            //client.println("Content-Length:  30");
            client.println("Connection: keep-alive");
            client.println("\r"); // CRLF terminates header 
 
            String json = "{";       
            for (int i = 0; i < 2; i++) {
              if ( SW_state(i) ) {
                json += "\"switch_" + String(i) + "\":1 ";
              } 
              else {
                json += "\"switch_" + String(i) + "\":0 ";
              } 
              if ( i < 1 ) json += ",";
            }
            json += "}";
            client.println(json);
            client.flush();
#ifdef DEBUG
            Serial.println(json);
#endif
            break;          
          }
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsZeroLength = true;
          if (firstLine) {
            firstLine = false;
#ifdef DEBUG
            Serial.println("-->" + line + "<--");
#endif
            if ( line.indexOf( "/powerOn/0" ) >= 0 ) {
#ifdef DEBUG
              Serial.println("Setting Relay0 to ON");
#endif  
              SW(0, HIGH);
              httpResponse = 200;
            }
           else if (line.indexOf( "/powerOn/1" ) >= 0 ) {
#ifdef DEBUG
              Serial.println("Setting Relay1 to ON");
#endif
              SW(1, HIGH);
              httpResponse = 200;
            }
            else if (line.indexOf( "/powerOff/0" ) >= 0 ) {
#ifdef DEBUG
              Serial.println("Setting Relay0 to OFF");
#endif
              SW(0, LOW);
              httpResponse = 200;
            }
            else if (line.indexOf( "/powerOff/1" ) >= 0 ) {
#ifdef DEBUG
              Serial.println("Setting Relay1 to OFF");
#endif
              SW(1, LOW);
              httpResponse = 200;
            }
          } 
          
        } else if (c != '\r') { 
          // The client has sent a request of some kind
          currentLineIsZeroLength = false;
          if (firstLine)
          {
            line = line + c; // Copy the VERB and Resource Request 
          }
        }
      }
    } // end while (client connected) ...
    // give the web browser time to receive the data
    delay(200);
    // close the connection:
    client.stop();
 #ifdef DEBUG
    Serial.println("client disconnected");
 #endif
  }
  delay(500);
}

bool SW_state(byte num) {
  bool val;
  switch (num) {
    case 0: val = digitalRead(PIN_RELAY_0); break;
    case 1: val = digitalRead(PIN_RELAY_1); break;
  }
  return !val;
}

void SW(byte num, bool val) {
  val = !val;
  switch (num) {
    case 0: digitalWrite(PIN_RELAY_0, val);
      break;
    case 1: digitalWrite(PIN_RELAY_1, val);
      break;
  }
}
