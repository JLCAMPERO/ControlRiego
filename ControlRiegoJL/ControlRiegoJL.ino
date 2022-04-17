/*
    Script creado para el módulo de Control de Riego
    Author: José Luis Campero Romero
    Master Agricultura Digital
*/
//Incluimos estas librerías para poder conectar la placa ESP8266 y FIREBASE
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <FirebaseArduino.h>
// WIFI y claves
#define WIFI_SSID "CAMPERO"
#define WIFI_PASSWORD "jlcr3113"
// Nombre de la BBDD creada en FIREBASE donde nos conectamos y autentificación
#define FIREBASE_HOST "conexionarduino-508b3-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "r9lzPNfgtJF37fYrcATdqN7TLV5NJYL2SLy9kngC"


//Estoy convirtiendo las variables a tipo string
String sendTemp, postData;

//Declaramos el pin analógico en el que queremos que vaya conectado nuestro sensor
int rele = D4;
int sensor_pin = A0;
int porcentaje = 0;
int ledrojo = D7;
String interruptor="0"; // campo que se actualiza desde la app para activar el riego manual,
//Declaramos la variable salida que queremos mostrar
int output_value, output_value_led, output_value_rele ; // variable utilizadas para controlar los valores de salida

//Función principal de Arduino que se inicia al principio

void setup() {

  Serial.begin(115200);

  Serial.println("Iniciando comunicación \n\n");
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);     // Iniciamos el Led azul de la placa
  pinMode(rele, OUTPUT);
  pinMode(ledrojo, OUTPUT);


  //Aqui estamos iniciando la comunicación Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);//Intentamos conectarnos al Wifi
  Serial.print("Conectándose a ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) //!=Distinto de. Va a estar intentando conectarse.
  { Serial.print(".");
    delay(500);
  }

  Serial.print("Conectado a ");
  Serial.println(WIFI_SSID);
  Serial.print("Dirección IP : ");
  Serial.println(WiFi.localIP());    //Mostramos la IP local por Monitor Serial
  // Conectados a internet se realiza la conexión a FIREBASE
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  delay(30);

  
}



void loop() {


  output_value = analogRead(sensor_pin);
  output_value_led = digitalRead(ledrojo);
  output_value_rele = digitalRead(rele);

  //Importante la calibración del sensor.
  //Cuando se tomaron medidas sobre un suelo seco,
  //el valor del sensor fue 1,
  //y en el caso de agua pura superior 750.
  // consideramos que la tierra está seca y necesita agua cuando el valor del sensor se encuentra entre 0 y 300.
  // El interruptor se encuentra a 0 hasta que se activa a 1 desde la app.

  if ((output_value<=300) && (interruptor=="0")) { // Regamos se enciende el led y se activa la bomba
    digitalWrite(ledrojo,HIGH);
    digitalWrite(rele, HIGH); // se activa el relé para que se ponga en funcionamiento la bomba.
    // Modificamos el campo Regar en Firebase 
    Firebase.setFloat("Regar", 1); // indica que estamos regando de forma automatica
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }
  } else { // No riega hay suficiente humedad en el suelo
    digitalWrite(ledrojo,LOW);
    digitalWrite(rele, LOW);
    Firebase.setFloat("Regar", 0); // indica que no regamos
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
  
   interruptor = Firebase.getString("Interruptor");
   Serial.print("Interruptor: ");
   Serial.println(interruptor);

    if (interruptor == "1") {  // queremos activar el riego manual el control pasa a la app
        digitalWrite(ledrojo,HIGH);
        digitalWrite(rele, HIGH);
    }
 /*
  } else {
    digitalWrite(Bomba, HIGH);
    Firebase.setFloat("Regar", 1);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
*/
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
  Serial.print("LedRojo: ");
  Serial.println(output_value_led);
  delay(2000);
  Serial.print("Rele: ");
  Serial.println(output_value_rele);
  delay(2000);


  // set Float  value
  Firebase.setFloat("HumedadSuelo", porcentaje);
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }

    // set string value
    Firebase.setString("message", "Conectado  Arduino...");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());
      return;
    }
  
}
