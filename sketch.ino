#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "AndroidAPB1A6" //WiFi network's name
#define STAPSK  "03031996"      //WiFi network's password
#endif

//function declarations
void convertFileToString();
void stringParsing();
int sendDataToServer();
void clearData();

//global variables
String myfile;
String finalString;
char mybuffer[1024]={0};

const int port = 4444;//server port
byte server_ip[] = { 212, 179, 205, 15 };//server address 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("board is ready");
  
  //Connecting to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
 
}


void loop() {
  delay(1000);
  // put your main code here, to run repeatedly:
  //myfile ="";
  Serial.println("loop function started...");
  if(Serial.available()>0){
    convertFileToString();
    stringParsing();
    Serial.println("this is myfile string:\n");
    Serial.println(myfile);
    Serial.println("this is final string:");
    Serial.println(finalString);
    //send data to the server
    
    while(sendDataToServer()==-1);//keep sending the data untill a successful connection to the server
    
    clearData();// clear data from previous files
  }
  else {
    Serial.println("Waiting for a file");
  }
  delay(1000);
}

void convertFileToString(){
  int c = Serial.read();
  while( c != '\n' && c != -1 ) c= Serial.read();//skip the first line (the title)
  c = Serial.read();//skip the first '\n'
  c = Serial.read();//skip the second '\n'
  while(c != -1){
    if( (c >= 32 && c <= 126 )|| c == '\t') {//check if 'c' is a printable character
      myfile.concat((char)c);
      Serial.print((char)c);
    }
    else if(c==13) myfile.concat('\n'); // '\n' == 13
    c = Serial.read();
  }
}

void stringParsing(){
finalString = "";
int i=0,j=0;
String columns = String("name= id= age= serial_number= beat_per_minute= time= date=");
while(columns[i] != '\0'){
  while(columns[i] != ' ' && columns[i]!= 0){
    finalString += columns[i];
    i++;
  }
  while(myfile[j] != '\t' && myfile[j]!= 0){
    finalString += myfile[j];
    j++;
  }
  if(columns[i]!=0)i++; //skip the '\t'
  j++;
  finalString += ';';
}
finalString.toCharArray(mybuffer, 1024);
}

int sendDataToServer(){
  
  WiFiClient client;
  if (!client.connect(server_ip, port)) {
   Serial.println("Connection to host failed");
   delay(5000);
   return -1;
  }
 
  // This will send a string to the server
  Serial.println("sending data to server");
  if (client.connected()) {
    client.println(mybuffer); //data example: "name=adar;id=315524868;age=24;serial_number=123;beat_per_minute=70;time=10:00;date=12/09/2019;"
     Serial.println("Data sent");
    client.stop(); //close the connection
    Serial.println("Connection to server closed");
  }
  return 0;
}

void clearData(){ // clear all global variables
  for(int i=0;i<100;i++) mybuffer[i]=0;
  myfile ="";
  finalString="";;
}
