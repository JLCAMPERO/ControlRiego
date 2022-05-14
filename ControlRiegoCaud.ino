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

//Declaramos el pin analógico en el que queremos que vaya conectado nuestros sensores
int sensor_pin1 = A0;
//int sensor_pin2 = A2;
int porcentaje = 0;
int ledrojo = D7;

//Declaramos los pin digitales donde van a ir conectados los reles para controlar las bombas. rele->Maceta 1 y rele2->Maceta2
int rele1 = D1;
int rele2 = D2;

//Declaramos el pin A0 que es donde vamos a conectar el sensor de temperatura
//int sensorPinTemp= A0;

String interruptor="0"; // campo que se actualiza desde la app para activar el riego manual,
String maceta1="0"; // campo que se actualiza desde la app para activar el riego manual,
String maceta2="0"; // campo que se actualiza desde la app para activar el riego manual,
//Declaramos la variable salida que queremos mostrar
int output_value_sensor1, output_value_led, output_value_rele1 ; // variable utilizadas para controlar los valores de salida de la Maceta 1
int output_value_sensor2, output_value_rele2 ; // variable utilizadas para controlar los valores de salida de la Maceta 2

//Declaración del Caudalimetro.
int sensorPinCaud = D3;
const int measureInterval = 2500;
volatile int pulseConter;
// YF-S201
const float factorK = 7.5;

// Funciones Caudalímetro.

void ISRCountPulse()
{
   pulseConter++;
}
float GetFrequency()
{
   pulseConter = 0;
   interrupts();
   delay(measureInterval);
   noInterrupts();
   return (float)pulseConter * 1000 / measureInterval;
}


//Función principal de Arduino que se inicia al principio

void setup() {

  Serial.begin(115200);
  //attachInterrupt(digitalPinToInterrupt(sensorPinCaud), ISRCountPulse, RISING);

  Serial.println("Iniciando comunicación \n\n");
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);     // Iniciamos el Led azul de la placa
  pinMode(rele1, OUTPUT);
  pinMode(rele2, OUTPUT);
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

  // Calculamos el caudal
  // obtener frecuencia en Hz
  float frequency = GetFrequency();
  // calcular caudal L/min
  float flow_Lmin = frequency / factorK;
  Serial.print("Frecuencia: ");
  Serial.print(frequency, 0);
  Serial.print(" (Hz)\tCaudal: ");
  Serial.print(flow_Lmin, 3);
  Serial.println(" (L/min)");
  if (flow_Lmin > 0) {
	    Firebase.setFloat("Caudalimetro", flow_Lmin);
  }
  //================Esperando el ADC1115 para ampliar pines analógicos======================
  //int value = analogRead(sensorPinTemp); // recogemos el valor de la temperatura
  //float millivolts = (value / 1023.0) * 3300; //Ponemos 3000 porque esta placa tiene 3v
  //float val = millivolts / 10; 
  //sendTemp= String(val);
  // Almacenamos el valor de la temperatura en la BBDD
  //Firebase.setFloat("Temperatura", String(val));
  //=======================================================================================
  //================Esperando el ADC1115 para ampliar pines analógico======================
  // output_value_sensor2 = analogRead(sensor_pin2); 
  //=======================================================================================
  // Recogemos los valores de los sensores de humedad
  output_value_sensor1 = analogRead(sensor_pin1);

  output_value_led = digitalRead(ledrojo);
  output_value_rele1 = digitalRead(rele1);
  output_value_rele2 = digitalRead(rele2);

  //Importante la calibración del sensor.
  //Cuando se tomaron medidas sobre un suelo seco,
  //el valor del sensor fue 1,
  //y en el caso de agua pura superior 750.
  // consideramos que la tierra está seca y necesita agua cuando el valor del sensor se encuentra entre 0 y 300.
  // El interruptor se encuentra a 0 hasta que se activa a 1 desde la app.
  
  // Control de riego automático para la Maceta 1
  if ((output_value_sensor1<=600) && (interruptor=="0")) { // Regamos, se enciende el led y se activa la bomba
    digitalWrite(ledrojo,HIGH);
    digitalWrite(rele1, HIGH); // se activa el relé para que se ponga en funcionamiento la bomba.
    // Modificamos el campo Regar en Firebase y activamos la Maceta 1
    Firebase.setFloat("Regar", 1); // indica que estamos regando de forma automatica
    Firebase.setFloat("Maceta1", 1); // indica que estamos regando de forma automatica
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }
  } else  { // No riega hay suficiente humedad en el suelo
    if (interruptor == "0") {
    digitalWrite(ledrojo,LOW);
    digitalWrite(rele1, LOW);
    Firebase.setFloat("Regar", 0); // indica que no regamos
    Firebase.setFloat("Maceta1", 0); // desactivamos la Maceta1 
    
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /number failed:");
      Serial.println(Firebase.error());
      return;
    }
    }
  }

  // Control de riego automático para la Maceta 2 cuando tengamos el ADC1115 para conectar un nuevo sensor de humedad


   interruptor = Firebase.getString("Interruptor");
   maceta1 = Firebase.getString("Maceta1");
   maceta2 = Firebase.getString("Maceta2");
   Serial.print("Interruptor: ");
   Serial.println(interruptor);

    if (interruptor == "1") {  // queremos activar el riego manual el control pasa a la app
        digitalWrite(ledrojo,HIGH);
        if (maceta1 == "1") {
           digitalWrite(rele1, HIGH);
      	} else {
      		digitalWrite(rele1, LOW);
	      }
        if (maceta2  == "1") {
           digitalWrite(rele2, HIGH);
      	} else {
	      	digitalWrite(rele2, LOW);
	      }
    } else {
       if (Firebase.getString("Regar") == "0") {
           digitalWrite(rele1, LOW);
           digitalWrite(rele2, LOW);
       }
    }

  //Aquí estamos mostrando el valor por pantalla
  Serial.print("Salida del sensor : ");
  Serial.println(output_value_sensor1);
  //Mapeando los valores de humedad para pasarlo
  //a porcentaje
  porcentaje = map(output_value_sensor1, 790, 0, 100, 0);


  Serial.print("Humedad Sensor (%): ");
  Serial.print(porcentaje);
  Serial.println("%");
  delay(2000);
  Serial.print("LedRojo: ");
  Serial.println(output_value_led);
  delay(2000);
  Serial.print("Rele1: ");
  Serial.println(output_value_rele1);
  delay(2000);

  // Almacenamos el valor de la humedad del suelo de la Maceta 1
  // set Float  value
  Firebase.setFloat("HumedadSuelo", porcentaje);
  // handle error
  if (Firebase.failed()) {
    Serial.print("setting /number failed:");
    Serial.println(Firebase.error());
    return;
  }

    // set string value
    Firebase.setString("message", "Conectado  Arduino");
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());
      return;
    }
  
}
