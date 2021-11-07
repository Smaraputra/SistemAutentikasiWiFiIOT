//Web Server
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#include <EEPROM.h>
#include <time.h>

//Setup Wifi Modul
const char* APssid = "MandauNodeMCU";
const char* APpassword = "12345678";

//Set Tombol Reset (Hard)
int tombolPin = 4;
int hasilldr;
int lastStatusTombolHard = 1;

void eepromSetup() {
  EEPROM.begin(512);
}

int eepromMaxData() { 
  return EEPROM.read(0); 
}

//String eepromRead(int offset) {
//  int numData = eepromMaxData();
//  int currIndex = 1;
//  for (int i = 0; i < numData; i++) {
//    int currDataLength = EEPROM.read(currIndex++);
//    if (i != offset) {
//      currIndex = currIndex + currDataLength;
//    } else {
//      String data = "";
//      for (int j = 0; j < currDataLength; j++) {
//        data = data + char(EEPROM.read(currIndex++));
//      }
//      return data;
//    }
//  }
//}

void eepromWriteData(String data[], int numData) {
  int currIndex = 0;
  EEPROM.write(currIndex++, numData);
  for (int i = 0; i < numData; i++) {
    int numCurrData = data[i].length();
    EEPROM.write(currIndex++, numCurrData);
    for (int j = 0; j < data[i].length(); j++) {
      EEPROM.write(currIndex++, data[i][j]);
    }
  }
  EEPROM.commit();
}

void setup(void) {
  delay(2000);
  Serial.begin(9600);
  Serial.println("");
  pinMode(tombolPin, INPUT_PULLUP);
  eepromSetup();
  int maxData = eepromMaxData();
  if(maxData==0){
    Serial.println("--------SETUP--------");
    Serial.println("----EEPROM KOSONG----");
    Serial.println("Pengaturan Access Point.");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(APssid, APpassword);
    Serial.println("Access Point Terbuka");
    Serial.println("Webserver akan dibuka pada IP "+WiFi.softAPIP().toString());
    server.on("/", halamanSetup);
    server.on("/isidata", handleForm);
    server.begin();
    Serial.println("Webserver sudah dibuka!");
    Serial.println("-------SELESAI-------");
    
  }else if(maxData==2){
    Serial.println("--------SETUP--------");
    Serial.println("----EEPROM TERISI----");
    Serial.println("Pengaturan Koneksi Wifi.");
    WiFi.mode(WIFI_STA);

    String ssidRead;
    int lokasipSSID = 1;
    int panjangSSID = EEPROM.read(lokasipSSID);
    for(int i=lokasipSSID+1; i<=lokasipSSID+panjangSSID; i++){
      ssidRead = ssidRead + char(EEPROM.read(i));
    }
    Serial.println(ssidRead);

    String passRead;
    int lokasipPASS = lokasipSSID+panjangSSID+1;
    int panjangPASS = EEPROM.read(lokasipPASS);
    for(int i=lokasipPASS+1; i<=lokasipPASS+panjangPASS; i++){
      passRead = passRead + char(EEPROM.read(i));
    }
    Serial.println(passRead);
    
    Serial.println("Menghubungkan dengan WiFi.");
    WiFi.begin(ssidRead.c_str(),passRead.c_str());
    for(int i = 1; i<6 ; i++){
      if(WiFi.status() != WL_CONNECTED){
        delay(2000);
        Serial.print(".");
        if(i==5){
          Serial.println("");
          Serial.println("Kredensial Login Salah!");
          Serial.println("Mereset EEPROM dan NodeMCU.");
          handleReset();
        }
      }else{
        break;
      }
    }
    
    Serial.println("");
    Serial.print("WiFi Terkoneksi : ");
    Serial.println(ssidRead);
    Serial.print("IP Address : ");
    Serial.println(WiFi.localIP());
    
    Serial.println("Webserver akan dibuka pada IP "+WiFi.localIP().toString());
    server.on("/", halamanTerkoneksi);
    server.on("/reset", handleReset);
    server.begin();
    Serial.println("Webserver sudah dibuka!");
    Serial.println("-------SELESAI-------");
  }
  
}

void loop(void) {
  delay(50);
  if(eepromMaxData()==2){
    hasilldr = analogRead(A0);
    int currentStatusTombolHard = digitalRead(tombolPin);
    if(currentStatusTombolHard!=lastStatusTombolHard){
      Serial.println("-----Tombol Ditekan-----");
      if(currentStatusTombolHard==0){
        lastStatusTombolHard=currentStatusTombolHard;
        handleReset();
      }
      delay(50);
    }
  }
  server.handleClient();
}

void halamanSetup(){
  String clientIP = server.client().remoteIP().toString();
  String page = R"===(
      <!DOCTYPE html>
      <html lang="en">
      <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="widht=device-width, initial-scale=1.0">
        <style>
          div {
            border-radius: 5px;
            background-color: #f2f2f2;
            padding: 20px;
          }
          
          input[type=text], select {
            width: 100%;
            padding: 12px 20px;
            margin: 8px 0;
            display: inline-block;
            border: 1px solid #ccc;
            border-radius: 4px;
            box-sizing: border-box;
          }
          
          input[type=submit] {
            width: 100%;
            background-color: #4CAF50;
            color: white;
            padding: 14px 20px;
            margin: 8px 0;
            border: none;
            border-radius: 4px;
            cursor: pointer;
          }
          
          h1{text-align: center;}
          h4{text-align: center;}
          
        </style>
        <title>Sistem EEPROM Webserver</title>
      </head>
      <body>
        <h1>Sistem EEPROM Webserver</h1>
        <h4>WIFI Belum Di-SETUP</h4>
        <div>
          <form action="/isidata">
            SSID WIFI:<br>
            <input type="text" name="ssid" value="" placeholder="SSID Wifi..">
            <br>
            Password WIFI:<br>
            <input type="text" name="pass" value="" placeholder="Password Wifi..">
            <br><br>
            <input type="submit" value="Simpan">
          </form>
        </div>
      </body>)===";
  server.send(200, "text/html", page);
}

void halamanTerkoneksi(){
  String clientIP = server.client().remoteIP().toString();
  String tombol = "<button onclick=\"location.href = '/reset'\">Reset EEPROM</button>";
  String ldr = String(hasilldr);
  String page = R"===(
      <!DOCTYPE html>
      <html lang="en">
      <head>
        <meta charset="UTF-8">
        <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <meta name="viewport" content="widht=device-width, initial-scale=1.0">
        <style>
          
        h1{text-align: center;}
        h4{text-align: center;}
        button {
          background-color: #4CAF50;
          color: white;
          padding: 15px 32px;
          text-align: center;
          display: inline-block;
          font-size: 16px;
          margin: 4px 2px;
          cursor: pointer;
          width: 100%;
        }
        
        div {
          border-radius: 5px;
          background-color: #f2f2f2;
          padding: 20px;
        }
        </style>
        <script>
          function refresh(refreshPeriod) 
          {
            setTimeout("location.reload(true);", refreshPeriod);
          } 
          window.onload = refresh(2000);
        </script>
        <title>Sistem EEPROM Webserver</title>
      </head>
      <body>
        <h1>Sistem EEPROM Webserver</h1>
        <h4>WIFI Telah Di-SETUP dan Terkoneksi</h4>
        <h4>Tingkat Kecerahan Cahaya = )==="+ ldr + R"===(</h4>
        <div>
          <p>)==="+ tombol + R"===(</p>
        </div>
        
      </body>)===";
  server.send(200, "text/html", page);
}

void handleForm() {
  String loginWifi[2];
  loginWifi[0] = server.arg("ssid");
  loginWifi[1] = server.arg("pass");
  
  Serial.println("--------INPUT--------");
  Serial.print("SSID Wifi : ");
  Serial.println(loginWifi[0]);
  Serial.print("Password Wifi : ");
  Serial.println(loginWifi[1]);
  int numData;
  numData= sizeof(loginWifi) / sizeof(String);
  eepromWriteData(loginWifi, numData);
  Serial.println("-------SELESAI-------");
  
  server.sendHeader("Location", String("/"), true);
  server.send(302, "text/plain", "");
  ESP.restart();
}

void handleReset() {
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  Serial.println("-------EEPROM RESET-------");
  EEPROM.end();
  ESP.restart();
}
