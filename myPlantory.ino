/*
myPlantory.ino

author: Eden Candelas
@elmundoverdees
HackerSpace Monterrey

Gardening system basado en arduino.
Controla via web valvula para riego.
Realiza mediciones de luminositad, temperatura y humedad.

depende de etherne_myPlantory.ino


*/


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
#define DISPARO_LUZ 50
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

struct lampara{
    int id;
    int pin;
    enum estados status;
};

struct valores{
    int valorLuminosidad;
    float valorTemperatura;
    int valorHumedad;
} lecturas;

struct valvula valvula;
struct sensor sensor;
struct lampara lampara;

/*Sensor Temperatura*/

OneWire ourWire(TEMPERATURA_Pin);
DallasTemperature tempSensors(&ourWire); 

/*Sensor iluminacion*/

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_LOW, 1);

/*ETHERNET*/

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 4);
String HTTP_req;

EthernetServer server(80);


//tiempos envio de informacion (segundos)
unsigned long timer_post;
unsigned long tiempo_post = 60;

void setup()
{
//Inicializacion de la valvula 

    valvula.id = 0;
    valvula.pin = 8;
    valvula.status = INACTIVO;
    valvula.timer = 0;
    valvula.tiempo = 10;

    pinMode(valvula.pin, OUTPUT);
    digitalWrite(valvula.pin, HIGH);

//Inicializacion de sensor de humedad
    sensor.id = 0;
    sensor.status = SECO;
    sensor.valorActual = -1;
    sensor.valorAnterior = -1;

//Inicializacion de lampara
    lampara.id = 0;
    lampara.pin = 9;
    lampara.status = INACTIVO;

    pinMode(lampara.pin, OUTPUT);
    digitalWrite(lampara.pin, HIGH);

//Inicializacion de estructura de lecturas
    lecturas.valorHumedad = -1;
    lecturas.valorTemperatura = -1;
    lecturas.valorLuminosidad = -1;

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
    timer_post = millis();

}

void loop()
{   
// Envia el post con los datos del sensor cada periodo definido por tiempo_post
    if(timer(timer_post,tiempo_post)){

       lecturas.valorLuminosidad = lecturaLuminosidad();
       lecturas.valorTemperatura = lecturaTemperatura();
       lecturas.valorHumedad = lecturaHumedad();

        EthernetClient clientRequest;
        enviarRequest(clientRequest, &lecturas);
        clientRequest.stop();
        timer_post = millis();
    }

//Si hay una conexion externa (alguien llamando desde un browser o una app)
//Se responde ejecutando procesarCliente()
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


