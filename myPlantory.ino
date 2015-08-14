#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <SPI.h>
#include <Ethernet.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMPERATURA_Pin 2
#define DISPARO 1000
#define SENSOR_HUMEDAD_PIN 0 

enum estados{INACTIVO, ACTIVO};
enum estadosSensor {SECO, HUMEDO};

struct valvula{
    int id;
    int pin;
    enum estados status;
    unsigned long timer;
    unsigned long tiempo;
};

struct sensor{
    int id;
    enum estadosSensor status;
    int valorActual;
    int valorAnterior;
};

struct valores{
    int valorLuminosidad;
    int valorTemperatura;
    int valorHumedad;
} lecturas;

struct valvula valvula;
struct sensor sensor;

OneWire ourWire(TEMPERATURA_Pin);
DallasTemperature sensors(&ourWire); 

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
//tiempos lecturas Temperatura (segundos);
unsigned long timer_tsl;
unsigned long tiempo_tsl = 10;

//tiempos ejecucion RIEGO (segundos)
unsigned long timer_riego;
unsigned long tiempo_riego = 10;

//tiempos envio de informacion (segundos)
unsigned long timer_post;
unsigned long tiempo_post = 60;

/*ETHERNET*/

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 4);

EthernetServer server(80);

String HTTP_req;

void setup()
{
    valvula.id = 0;
    valvula.pin = 8;
    valvula.status = INACTIVO;
    valvula.timer = 0;
    valvula.tiempo = 10;

    sensor.id = 0;
    sensor.status = SECO;
    sensor.valorActual = -1;
    sensor.valorAnterior = -1;

    lecturas.valorHumedad = -1;
    lecturas.valorTemperatura = -1;
    lecturas.valorLuminosidad = -1;

    pinMode(valvula.pin, OUTPUT);
    digitalWrite(valvula.pin, LOW);
    Serial.begin(9600);
    Serial.println("<<INIT>>");

    configureEthernet();

    if(!tsl.begin())
        {
        Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
        while(1);
    }

    configureLuxSensor();
  
    Serial.println("");
    timer_tsl = millis();
    timer_riego = millis();
    timer_post = millis();

}

void loop()
{   

    if(timer(timer_post,tiempo_post)){

       lecturas.valorLuminosidad = lecturaLuminosidad();
       lecturas.valorTemperatura = lecturaTemperatura();
       lecturas.valorHumedad = lecturaHumedad();

        EthernetClient clientRequest;
        enviarRequest(clientRequest, &lecturas);
        clientRequest.stop();
        timer_post = millis();
    }

    EthernetClient clientServer = server.available();
    if (clientServer){

        lecturas.valorLuminosidad = lecturaLuminosidad();
        lecturas.valorTemperatura = lecturaTemperatura();
        lecturas.valorHumedad = lecturaHumedad();
        procesarCliente(clientServer, &lecturas);
    }

}

int timer(unsigned long inicio,  unsigned long limite){
    unsigned long actual = millis(); 
    if((actual - inicio) > (limite*1000)){
        Serial.println(inicio);
        Serial.println(actual);
        Serial.print("Timer>");
        Serial.println("excedido");
        return 1;
    }
    return 0;
}

void configureLuxSensor(void){
        tsl.setGain(TSL2561_GAIN_1X);
        tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
        Serial.println("-------------------------------------");
        Serial.print  ("Gain:       "); Serial.println("Auto");
        Serial.print  ("Timing:     "); Serial.println("13 ms");
        Serial.println("-------------------------------------");
        
}

int lecturaLuminosidad(){
    sensors_event_t event;
    tsl.getEvent(&event);

    if (event.light){
        Serial.print(event.light); Serial.println(" lux");
    }else{
        Serial.println("Sensor overload");
    }

    return event.light;
}

float lecturaTemperatura(){
    sensors.requestTemperatures(); 
    float temp= sensors.getTempCByIndex(0);
    Serial.print("VALOR TEMPERATURA> ");
    Serial.print(temp); 
    Serial.println(" grados Centigrados");
    return temp;
}

void ejecutarRiego(){
    sensor.valorActual = analogRead(SENSOR_HUMEDAD_PIN);
    Serial.print("VALOR>");
    Serial.println(sensor.valorActual);
    delay(1000);

    if(sensor.valorActual < DISPARO && sensor.valorAnterior > DISPARO){
        sensor.status = HUMEDO;
        digitalWrite(valvula.pin, HIGH);
        Serial.print("STATUS>");
        Serial.println("HUMEDO");
        sensor.valorAnterior = sensor.valorActual;
    }else if(sensor.valorActual > DISPARO && sensor.valorAnterior < DISPARO){
        sensor.status = SECO;
        digitalWrite(valvula.pin, LOW);
        Serial.print("STATUS>");
        Serial.println("SECO");
        valvula.timer = millis();
        while(!timer(valvula.timer, valvula.tiempo)){
        //continue
            delay(1000);
            Serial.println("STATUS>");
            Serial.println("REGANDO");
        }
        digitalWrite(valvula.pin, HIGH);
        Serial.println("STATUS>");
        Serial.println("DEJE DE REGAR");
        sensor.valorAnterior = sensor.valorActual;
    }else {
        Serial.print(" STATUS>");
        Serial.println("NADA A EJECUTAR");
    }  
}

int lecturaHumedad(){
    sensor.valorActual = analogRead(SENSOR_HUMEDAD_PIN);
    Serial.print("VALOR HUMEDAD> ");
    Serial.println(sensor.valorActual);
    delay(1000);
    return sensor.valorActual;

}

void enviarRequest(EthernetClient client, struct valores *lecturas){
  // if there's a successful connection:
  char serverAddress[] = "www.arduino.cc";  

  if (client.connect(serverAddress, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /latest.txt HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    //lastConnectionTime = millis();
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}

void procesarCliente(EthernetClient client, struct valores *lecturas){
    Serial.print("ETHERNET>");
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        HTTP_req += c;

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            //client.println("Refresh: 5");  // refresh the page automatically every 5 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            
            // Temperatura
            client.print("Temperatura : ");
            client.print(lecturas->valorTemperatura);
            client.println("<br />");  
            //Luminosidad          
            client.print("Luminosidad :");
            client.print(lecturas->valorLuminosidad);
            client.println("<br />"); 
            //Humedad           
            client.print("Humedad : ");
            client.print(lecturas->valorHumedad);
            client.println("<br />");
            client.println("</html>");

            processSubmit(client);

            HTTP_req = "";
            break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
}

void processSubmit(EthernetClient cl){

    cl.println("<form action=\"action_page.php\" method=\"GET\">");
    cl.println("<input type=\"submit\" name=\"regar\"value=\"on\">");
    cl.println("<form/>");

    if (HTTP_req.indexOf("regar=on") > -1){
        Serial.println("DEBE REGAR");
    }
}

void configureEthernet(){
// start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("ETHERNET> server is at");
  Serial.println(Ethernet.localIP());


}