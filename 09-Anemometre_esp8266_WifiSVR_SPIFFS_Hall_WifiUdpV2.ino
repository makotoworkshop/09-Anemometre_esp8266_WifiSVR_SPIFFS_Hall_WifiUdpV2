#include <ESP8266WebServer.h>
#include <FS.h>
#include <WiFiUdp.h>  //pour upload de pgm Arduino via wifi
#include <ArduinoOTA.h> //pour upload de pgm Arduino via wifi

/****************/
/* DÉCLARATIONS */
/****************/
ESP8266WebServer server ( 8080 );   // on instancie un serveur ecoutant sur le port 80
#define pinHallAnemo D2   // le capteur à effet Hall est connecté à la pin D2
#define ssid      "xxxx"    // WiFi SSID
#define password  "****"  // WiFi password
unsigned long rpmVent = 0;
unsigned long vitVentKMH = 0;
unsigned long dateDernierChangementVent = 0;
unsigned long dateDernierChangementKMH = 0;
float intervalleKMH = 0;

/*********/
/* SETUP */
/*********/
void setup() {
  Serial.begin ( 115200 );    // init du mode débug
  // Connexion au WiFi
  WiFi.begin ( ssid, password );
  // Attente de la connexion au réseau WiFi / Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 ); 
    Serial.print ( "." );
  }
  // Connexion WiFi établie
  Serial.println ( "" );
  Serial.print ( "Connected to " ); Serial.println ( ssid );
  Serial.print ( "IP address: " ); Serial.println ( WiFi.localIP() );

  // Montage de la zone mémoire SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount failed");
  } 
  else {
    Serial.println("SPIFFS Mount succesfull");
  }
  delay(50);

  // Pin capteurs
  attachInterrupt(pinHallAnemo, rpm_vent, FALLING); 
  
  // Pages web du serveur
  server.serveStatic("/js", SPIFFS, "/js");       // dossier js qui contient les fichiers JavaScripts
  server.serveStatic("/css", SPIFFS, "/css");     // dossier css qui contient les fichiers css
  server.serveStatic("/", SPIFFS, "/index.html"); // racine du serveur, pointe l'index.html
  server.on("/mesures.json", sendMesures);        // écrit le fichier json à l'appel de la fonction
  server.begin();                                 // démarre le serveur
  Serial.println ( "HTTP server started" );
  ArduinoOTA.setHostname("AnemometreWiFi");   // on donne une petit nom a notre module, pour upload de pgm Arduino via wifi
  // ArduinoOTA.setPassword((const char *)"1357");
  ArduinoOTA.begin();   // initialisation de l'OTA, pour upload de pgm Arduino via wifi
}

/*************/
/* FONCTIONS */
/*************/
void rpm_vent()   // appelée par l'interruption, Anémomètre vitesse du vent.
{ 
  unsigned long dateCourante = millis();
  intervalleKMH = (dateCourante - dateDernierChangementVent);
  Serial.print ( "intervalle en s : " );
  Serial.println (intervalleKMH/1000); // affiche l'intervalle de temps entre deux passages
  if (intervalleKMH != 0)  // attention si intervalle = 0, division par zero -> erreur
  {
    rpmVent = 60 / (intervalleKMH /1000);  
  }
  vitVentKMH = ( rpmVent + 6.174 ) / 8.367;
  Serial.print ( "vitVentKMH : " );
  Serial.println ( vitVentKMH ); // affiche les rpm  
  Serial.println ( "" );
  dateDernierChangementVent = dateCourante;
}


void sendMesures()    // appelée par le serveur web
{  
 String json = "{\"rpm\":\"" + String(vitVentKMH) + "\"}";
// prépare et formate la valeur pour le fichier json sous la forme : {"VitesseVent":"0.00"}
//                          {
//                            "VitesseVent":"25"
//                          }
  server.send(200, "application/json", json);   // envoie dans le valeur dans le fichier json qui tourne en mémoire
//  Serial.println("Mesures envoyees");
}


void RemiseZeroVitVentKMH ()
{
  unsigned long dateCouranteKMH = millis();
  if (intervalleKMH == intervalleKMH) // Si ça ne tourne plus (valeur plus mise à jour)
  {  
    float dureeKMH = (dateCouranteKMH - dateDernierChangementKMH);
    if (dureeKMH > 10000) // Si ça ne tourne plus depuis 10 secondes
    {
      Serial.print ( "dureeKMH : " );
      Serial.println ( dureeKMH ); // affiche les rpm  
      vitVentKMH = 0;  // Remsise à zero !
      dateDernierChangementKMH = dateCouranteKMH;    
    }
  }
}

/*************/
/* PROGRAMME */
/*************/
void loop()
{
  server.handleClient();  // à chaque iteration, on appelle handleClient pour que les requetes soient traitees
  RemiseZeroVitVentKMH ();
  delay(100); // la boucle fait tourner sendMesures(), via handleClient, régler delais si besoin de mettre à jour le JSON qu'à une fréquence voulue plutôt qu'instantanément 
  ArduinoOTA.handle();  // régler upload speed à 9600 : verifie si un upload de programme Arduino est envoyé sur l'ESP8266
}
