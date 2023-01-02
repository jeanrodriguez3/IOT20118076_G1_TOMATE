#include <ArduinoJson.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#define BUILTIN_LED 2
#define DHTPIN 14
#define DHTTYPE DHT11

// Variables con datos para la conexion a la red

const char* ssid = "LOS JARDINES";
const char* password = "orizon123";
const char* mqtt_server = "broker.emqx.io";
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
DHT dht(DHTPIN, DHTTYPE);
int value = 0;
String ESTADO;
const int relayPin=33;
bool releState=false; 

void setup_wifi() {

  delay(10);
  // Empezamos conectándonos a una red WiFi
  Serial.println();
  Serial.print("Conectando ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  randomSeed(micros());
  //Imprime el ip address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Encender el LED si se recibió un 1 como primer carácter
  if ((char)payload[0] == 'A') {
    digitalWrite(BUILTIN_LED, LOW);   // Encienda el LED (tenga en cuenta que BAJO es el nivel de voltaje
    releState=false;
    
    // está activo bajo en el ESP32
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Apaga el LED haciendo que el voltaje sea ALTO
    releState=true;
    
  }

}

void reconnect() {
  // Bucle hasta que estemos reconectados
  while (!client.connected()) {
    Serial.print("Intentando la conexión MQTT...");
    // Crear una identificación de cliente aleatoriaID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // intento de conexión
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      // Una vez conectado, publicar un anuncio...
      client.publish("Sensores", "Estas conectado al servidor mqtt");
      // se suscribe
      client.subscribe("ESP32");
    } else {
      Serial.print("fallido, rc=");
      Serial.print(client.state());
      Serial.println("inténtalo de nuevo en 5 segundos");
      // Espere 5 segundos antes de volver a intentarlo
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Inicializa el led del esp32 en output
  pinMode(relayPin,OUTPUT);         //Inicializa el pin del rele en output
  digitalWrite(releState,HIGH);
  Serial.begin(115200);
  setup_wifi();//Se conecta a la red con SSID  y contraseña
  client.setServer(mqtt_server, 1883);//Se conecta al servidor 
  client.setCallback(callback);
   // Read humidity
    float h = dht.readHumidity();
    // Read temperature as Celsius
    float t = dht.readTemperature();
  delay(100);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
   
    // Lee la humedad
    float h = dht.readHumidity();
    // Lee la temp en Celsius
    float t = dht.readTemperature();
    //Condiciones para la activacion del riego
      if (t >= 25 && h <= 20){
      ESTADO="REGANDO";
      digitalWrite(relayPin , LOW);
      } else if (t >= 36 && h <= 65 ){
        digitalWrite(relayPin , LOW);
         ESTADO = "REGANDO";
      }else if(releState){
          
          ESTADO = "REGANDO";
          digitalWrite(relayPin , LOW);
      }else{
        ESTADO = "APAGADO";
        digitalWrite(relayPin,HIGH);
      }
      //Se crea el json con los datos del sensor
    StaticJsonDocument<128> doc;

    doc["DEVICE"] = "ESP32";
    //doc["Anho"] = 2020;
    //doc["Empresa"] = "Doge";
    doc["TEMPERATURA"] = t;
    doc["HUMEDAD"] = h;
    doc["ESTADO"] = ESTADO;

    String output;
    //Serializa los datos del json 
    serializeJson(doc, output);
    
    Serial.print("Publish message: ");
    Serial.println(output);
    //Se envian los datos serializados al servidor mosquito en el topico Sensores
    client.publish("Sensores",output.c_str());

    



  }
}
