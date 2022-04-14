//Script creado para el módulo de riego
//Author: José Luis Campero Romero 
//Master Agricultura Digital
/*
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>

// Set these to run example.
#define FIREBASE_HOST "controlriego-a244c-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "your-key"
#define WIFI_SSID "your-wifi"
#define WIFI_PASSWORD "your-pass-wifi"
*/
//Declaramos el pin analógico en el que queremos que vaya conectado nuestro sensor
int Bomba = 8;
int sensor_pin = A0;
int porcentaje = 0; 
int ledrojo=13;
//Declaramos la variable salida que queremos mostrar
int output_value ;
//Función principal de Arduino que se inicia al principio
void setup() {
  
  Serial.begin(9600);
  pinMode(ledrojo,OUTPUT);
  pinMode(Bomba, OUTPUT);
  Serial.println("Leyendo el sensor ...");
  delay(2000);
  }

//Función Loop que se repide constantemente
void loop() {

  output_value= analogRead(sensor_pin);
  //Importante la calibración del sensor. 
  //Cuando se tomaron medidas sobre un suelo seco, 
  //el valor del sensor fue 0, 
  //y en el caso de agua pura superior 790. 
   // digitalWrite(ledrojo,HIGH);
  if (output_value>0) {
    digitalWrite(ledrojo,HIGH);
    // digitalWrite(Bomba, HIGH);
  } else {
    digitalWrite(ledrojo,LOW);
  }
  if(output_value >= 100){   
    digitalWrite(Bomba, LOW);
  } else{
    digitalWrite(Bomba, HIGH);
  }

  //Aquí estamos mostrando el valor por pantalla
  Serial.print("Salida del sensor : ");
  Serial.println(output_value);
  //Mapeando los valores de humedad para pasarlo 
  //a porcentaje
  porcentaje = map(output_value, 790, 0, 100, 0);
  
  
  Serial.print("Humedad (%): ");
  Serial.print(porcentaje);
  Serial.println("%");
  delay(2000);
  }
