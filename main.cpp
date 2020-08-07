#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

//SSID and Password of your WiFi router
const char* ssid = "your_ssid_name";
const char* password = "your_wifi_password";

ESP8266WebServer server(80);

//LED Connections
const int RedLED = 13; 
const int GreenLED = 12; 
const int BlueLED = 14;  
const int RedLED2 = 16;  
const int GreenLED2 = 5; 
bool multipleRandom = false;
const int BlueLED2 = 4; 

String setcolor = "#ff00ff"; //Set color for HTML

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>Mood Lamp (RGB LED Color) Controller</title>
</head>
<body style="background:@@color@@;">

<center>
    Mood Lamp (RGB LED Color) Controller<br><br><br>
  <form method="post" action="/form">
      Color:
      <input type="color" name="color" value="@@color@@">
      <button type="submit" name="state" value="stop">SET</button>
  </form>
  <form method="post" action="/randomOnce">
      <button type="submit" name="state" value="stop">Random Color</button>
  </form>
  <form method="post" action="/randomMultiple">
      <button type="submit" name="state" value="stop">Change Random Color</button>
  </form>
</center>
</body>
</html>
)=====";

//=======================================================================
//                    handles main page
//=======================================================================
void handleRoot() {
  String p = MAIN_page;  
  p.replace("@@color@@",setcolor);    //Set page background color and selected color
  server.send(200, "text/html", p);    
}

//=======================================================================
//                    Handle Set Color
//=======================================================================
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
void randomOnce(){
  multipleRandom = false;
  int redRandom = random(255);
  int greenRandom = random(255);
  int blueRandom = random(255);
  byte R, G, B;
  R = redRandom;
  G = greenRandom;
  B = blueRandom;
  char hex[7] = {0};
  sprintf(hex,"%02X%02X%02X",R,G,B); //convert to an hexadecimal string. Lookup sprintf for what %02X means.
  String color = "#";
  color += hex;
  setcolor = color; //Store actual color set for updating in HTML
  analogWrite(RedLED, 1024-redRandom*4); // R off
  analogWrite(GreenLED, 1024-greenRandom*4); // B off
  analogWrite(BlueLED, 1024-blueRandom*4); // G off
  analogWrite(RedLED2, 1024-redRandom*4); // R off
  analogWrite(GreenLED2, 1024-greenRandom*4); // B off
  analogWrite(BlueLED2, 1024-blueRandom*4); // G off
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated-- Press Back Button");
}
void randomMultiple(){
  multipleRandom = true;
}
void handleRgb() {
  multipleRandom = false;
  String color = server.arg("color");
  int redA = getValue(color, 'x', 0 ).toInt();
  int greenA = getValue(color, 'x', 1 ).toInt();
  int blueA = getValue(color, 'x', 2 ).toInt();
  int red = 1024 - redA*4;
  int green = 1024 - greenA*4;
  int blue = 1024 - blueA*4;
  byte R, G, B;
  R = redA;
  G = greenA;
  B = blueA;
  char hex[7] = {0};
  sprintf(hex,"%02X%02X%02X",R,G,B); //convert to an hexadecimal string. Lookup sprintf for what %02X means.
  color = "#";
  color += hex;
  setcolor = color; //Store actual color set for updating in HTML
  
  Serial.print(color);
  Serial.print("r, g, b: ");
  Serial.print(red);
  Serial.print(" , ");
  Serial.print(green);
  Serial.print(" , ");
  Serial.println(blue);
  analogWrite(RedLED, red); // R off
  analogWrite(GreenLED, green); // B off
  analogWrite(BlueLED, blue); // G off
  analogWrite(RedLED2, red); // R off
  analogWrite(GreenLED2, green); // B off
  analogWrite(BlueLED2, blue); // G off
}
void handleForm() {
  multipleRandom = false;
  String color = server.arg("color");
  //form?color=%23ff0000
  setcolor = color; //Store actual color set for updating in HTML
  Serial.println(color);

  //See what we have recived
  //We get #RRGGBB in hex string

  // Get rid of '#' and convert it to integer, Long as we have three 8-bit i.e. 24-bit values
  long number = (int) strtol( &color[1], NULL, 16);

  //Split them up into r, g, b values
  long r = number >> 16;
  long g = (number >> 8) & 0xFF;
  long b = number & 0xFF;
  
  //PWM Correction
  r = r * 4; 
  g = g * 4;
  b = b * 4;
//for ULN2003 or Common Cathode RGB LED not needed

  r = 1024 - r;
  g = 1024 - g;
  b = 1024 - b;

  //ESP supports analogWrite All IOs are PWM
  analogWrite(RedLED,r);
  analogWrite(GreenLED,g);
  analogWrite(BlueLED,b);
  analogWrite(RedLED2,r);
  analogWrite(GreenLED2,g);
  analogWrite(BlueLED2,b);

  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "Updated-- Press Back Button");

  delay(500);  
}
//=======================================================================
//                    SETUP
//=======================================================================
void setup(){
  Serial.begin(115200);   //Start serial connection  
  
  pinMode(RedLED,OUTPUT);
  pinMode(GreenLED,OUTPUT);
  pinMode(BlueLED,OUTPUT);
  Serial.println("WIFIYI BAGLICAM GALIBA");
  
    WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("WIFIYI BAGLADIM GALIBA");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  
  server.on("/", handleRoot);  //Associate handler function to path
  server.on("/form",handleForm);
  server.on("/rgb", handleRgb);
  server.on("/randomOnce", randomOnce);
  server.on("/randomMultiple", randomMultiple);
    
  server.begin();                           //Start server
  Serial.println("HTTP server started");
}
//=======================================================================
//                    LOOP
//=======================================================================
void loop(){
  server.handleClient();
  if(multipleRandom){
    int redRandom = random(1024);
    int greenRandom = random(1024);
    int blueRandom = random(1024);
    analogWrite(RedLED, redRandom); // R off
    analogWrite(GreenLED, greenRandom); // B off
    analogWrite(BlueLED, blueRandom); // G off
    analogWrite(RedLED2, redRandom); // R off
    analogWrite(GreenLED2, greenRandom); // B off
    analogWrite(BlueLED2, blueRandom); // G off
    byte R, G, B;
    R = redRandom/4;
    G = greenRandom/4;
    B = blueRandom/4;
    char hex[7] = {0};
    sprintf(hex,"%02X%02X%02X",R,G,B); //convert to an hexadecimal string. Lookup sprintf for what %02X means.
    String color = "#";
    color += hex;
    setcolor = color; //Store actual color set for updating in HTML
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "Updated-- Press Back Button");
    delay(1000);
  }
}
