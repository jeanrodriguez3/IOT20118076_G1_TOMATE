#include <WiFi.h>
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
// Reemplace con sus credenciales de red
const char* ssid     = "ESP32";
const char* password = "12345678";

// Establezca el número de puerto del servidor web en 80
WiFiServer server(80);

// Variable para almacenar la solicitud HTTP
String header;
String ESTADO;
const int relayPin=33;
bool releState=false; 

void setup() {
  Serial.begin(115200);

  pinMode(relayPin,OUTPUT);
  digitalWrite(releState,HIGH);
  // Conéctese a la red Wi-Fi con SSID y contraseña
  Serial.print("Configuración de AP (Punto de acceso)...");
  // Conéctese a la red Wi-Fi con SSID y contraseña
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Escuche los clientes entrantes
     // Leer humedad
    float h = dht.readHumidity();
    // Leer la temperatura como Celsius
    float t = dht.readTemperature();
    String t1= String(t);
    String h1= String(h);
  if (client) {                             // Si un nuevo cliente se conecta,
    Serial.println("New Client.");          // imprimir un mensaje en el puerto serie
    String currentLine = "";                // hacer una cadena para contener los datos entrantes del cliente
    while (client.connected()) {            // Bucle mientras el cliente está conectado
      if (client.available()) {             // Si hay bytes para leer del cliente,
        char c = client.read();             // leer un byte, entonces
        Serial.write(c);                    // imprimirlo en el monitor serie
        header += c;
        if (c == '\n') {                    // si el byte es un carácter de nueva línea
          // si la línea actual está en blanco, tienes dos caracteres de nueva línea seguidos.
          // ese es el final de la solicitud HTTP del cliente, así que envíe una respuesta:
          if (currentLine.length() == 0) {
            // Los encabezados HTTP siempre comienzan con un código de respuesta (por ejemplo, HTTP/1.1 200 OK)
             // y un tipo de contenido para que el cliente sepa lo que viene, luego una línea en blanco:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // enciende y apaga los GPIO
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              ESTADO = "on";
              releState=true;
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              ESTADO = "off";
              
              releState=false;
            } 
            
            // Mostrar la página web HTML
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS para diseñar los botones de encendido/apagado
            
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Encabezado de página web
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Muestra el estado actual y los botones ON/OFF para GPIO 26 
            client.println("<p>rele - ESTADO " + ESTADO + "</p>");
            // Si output26State está apagado, muestra el botón ON     
            if (ESTADO=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<h2>Temperatura "+t1+"C</h2>");
            client.println("<h2>Humedad "+h1+"%</h2>");
            // La respuesta HTTP termina con otra línea en blanco
            client.println();
            // Salir del bucle while
            break;
          } else { // si tiene una nueva línea, borre currentLine
            currentLine = "";
          }
        } else if (c != '\r') { // si obtuvo algo más que un carácter de retorno de carro,
          currentLine += c;     // agregarlo al final de currentLine
        }
      }
    }
    // Borrar la variable de encabezado
    header = "";
    //Cerrar la conexión
    client.stop();
    Serial.println("Cliente desconectado.");
    Serial.println("");
  }
  if (t >= 25 && h <= 20){
      
      digitalWrite(relayPin , LOW);
      } else if (t >= 36 && h <= 65 ){
        digitalWrite(relayPin , LOW);
         
      }else if(releState){
          
         
          digitalWrite(relayPin , LOW);
      }else{
        
        digitalWrite(relayPin,HIGH);
      }
}
