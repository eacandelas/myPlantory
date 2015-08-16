/*sensores_myPlantory.ino



*/

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

void ejecutarLampara(){
    int luminosidad = lecturaLuminosidad();
    Serial.print("VALOR LUX: ");
    Serial.print(luminosidad);

    if (luminosidad < DISPARO_LUZ){
        digitalWrite(lampara.pin, LOW);
        Serial.println("LAMPARA ENCENDIDA");
    }else if (luminosidad > DISPARO_LUZ){
        digitalWrite(lampara.pin, HIGH);
        Serial.println("NO SE NECESITA ENCENDER LAMPARA");
    }else{
        digitalWrite(lampara.pin, HIGH);
        Serial.println("APAGADA");
    }
}

float lecturaTemperatura(){
    tempSensors.requestTemperatures(); 
    float temp= tempSensors.getTempCByIndex(0);
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

    if(sensor.valorActual > DISPARO){
        sensor.status = SECO;
        digitalWrite(valvula.pin, LOW);
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
    }else {
        Serial.print("STATUS>");
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

