#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#define SCHERM_BREEDTE 128  // OLED display breedte, in pixels
#define SCHERM_HOOGTE 64    // OLED display hoogte, in pixels
#define OLED_RESET -1
#define SCHERM_ADRES 0x3C
Adafruit_SSD1306 display(SCHERM_BREEDTE, SCHERM_HOOGTE, &Wire, OLED_RESET);
WiFiClientSecure client;
DynamicJsonDocument jsonObject(2048);

const char* ssid = "duohotspot";     // Vul hier jouw eigen netwerknaam (ssid) in
const char* password = "workshop";  // Vul hier jouw eigen netwerkwachtwoord in
const String server = "workshopduo.am0.nl"; // Het adres van de server
const String urlDatum = "/?type=datum"; // De url waar de informatie te vinden is
const String urlTijd = "/?type=tijd"; // De url waar de informatie te vinden is

int huidig_scherm = 0;
int aantal_schermen = 2;
bool button_pressed = false;
String datum;
String tijd;

void setup() {
  Serial.begin(9600);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCHERM_ADRES)) {
    // Deze eeuwigdurende lus zorgt ervoor dat de applicatie niet verder gaat bij een fout
    for (;;)
      ;
  }
  maakVerbindingMetWifi();
}

// Deze functie maakt verbinding met het netwerk
void maakVerbindingMetWifi() {
  WiFi.mode(WIFI_STA);                     // Type wifi verbinding instellen
  WiFi.begin(ssid, password);              // Gebruik WiFi.begin om verbinding te maken
  while (WiFi.status() != WL_CONNECTED) {  // Herhalen tot er verbinding is
    display.print(".");                    // Schrijf een punt op het scherm
    display.display();                     // Toon de teksten op het scherm
    delay(500);                            // Wacht een halve seconde
  }
  client.setInsecure();
}

void clearDisplay() {
  // Maak het scherm leeg
  display.clearDisplay();
  display.setTextSize(1);  // Zet het tekstformaat op 2 (2x zo groot)
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
}

void printPaginaDatum(String input) {
  clearDisplay();
  display.printf("Datum: %s", input);
  // Laat hetgeen we getekend hebben zien op het scherm
  display.display();
}

void printPaginaTijd(String input) {
  clearDisplay();
  display.printf("Tijd: %s", input);
  // Laat hetgeen we getekend hebben zien op het scherm
  display.display();
}

void doeApiCalls(String url) {
  display.clearDisplay();                 // Maak het scherm leeg
  display.setTextSize(1);                 // Zet het tekstformaat op 1
  display.setCursor(0, 0);                // Begin met schrijven op positie 0,0 (links boven)
  // Uitvoeren als er een verbinding gemaakt is met de server
  if (client.connect(server, 443)) {
    display.print("Gegevens ophalen...");                 // Schrijf tekst naar het scherm
    // Uitvoeren als er een fout is opgetreden
  } else {
    display.print("Kan geen verbinding maken met server"); // Schrijf tekst naar het scherm
  }
  display.display();
  // Toon de tekst op het scherm
  client.println("GET https://" + server + url + " HTTP/1.0");
  client.println("Host: " + server);
  client.println("Connection: close");
  client.println();

  // Wachten tot de server klaar is om de gegevens te versturen
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      break;
    }
  }
  String gegevens = client.readString();  // De gegevens van de server uitlezen
  deserializeJson(jsonObject, gegevens);  // De gegevens omzetten naar een JSON object
  // De huidige prijs uitlezen
  Serial.println(gegevens);
  if (url == urlDatum) {
    datum = jsonObject["datum"].as<String>();
  } else {
    String uren = jsonObject["tijd"]["uren"].as<String>();
    String minuten = jsonObject["tijd"]["minuten"].as<String>();
    tijd = uren + ":" + minuten;
  }
  client.stop();
}

void loop() {
  if (digitalRead(D3) == LOW && !button_pressed) {
    huidig_scherm = (huidig_scherm + 1) % aantal_schermen;
    button_pressed = true;

    if (huidig_scherm == 1) {
      doeApiCalls(urlDatum);
      printPaginaDatum(datum);
    } else {
      doeApiCalls(urlTijd);
      printPaginaTijd(tijd);
    }
  } else if (digitalRead(D3) == HIGH) {
    button_pressed = false;
  }
}
