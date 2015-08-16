/*sensores_myPlantory.ino



*/

void configureLuxSensor(void){
//Asigna los valor iniciales al sensor de luminosidad para que regrese valores correctos

        tsl.setGain(TSL2561_GAIN_1X);
        tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
        Serial.println("-------------------------------------");
        Serial.print  ("Gain:       "); Serial.println("Auto");
        Serial.print  ("Timing:     "); Serial.println("13 ms");
        Serial.println("-------------------------------------");
        
}


int lecturaLuminosidad(){
/*
Esta funcion regresa el valor de luminosidad en base al objeto tsl
*/    
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
/*
Revisa si el valor de luminosidad es menor al disparo de la luz.
En caso de que asi sea activa el pin de la lampara (se activa en LOW)
En caso que el valor del a misma sea alto evita que la lampara se encienda

DISPARO_LUZ es el valor limite que debe de obtener el sensor para su funcionamiento

*/


    int luminosidad = lecturaLuminosidad();
    Serial.print("VALOR LUX: ");
    Serial.println(luminosidad);

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

void ejecutarLamparaOff(){
/*
Apagamos lampara al momento, no se realiza validacion
*/

    digitalWrite(lampara.pin, HIGH);
    Serial.println("LAMPARA APAGADA");
}

float lecturaTemperatura(){
//Regresa el valor de temperatura en grados centigrados.
    tempSensors.requestTemperatures(); 
    float temp= tempSensors.getTempCByIndex(0);
    Serial.print("VALOR TEMPERATURA> ");
    Serial.print(temp); 
    Serial.println(" grados Centigrados");
    return temp;
}

void ejecutarRiego(){
/*
Se toma el valor de humedad y si esta por arriba del valor de DISPARO activa la valvula
por el tiempo definido en valvula.tiempo, cuando ese tiempo ha transcurrido, la valvula 
se cierra.
Si el valor de humedad es menor al disparo, no se activa nada
*/
    sensor.valorActual = analogRead(SENSOR_HUMEDAD_PIN);
    Serial.print("VALOR>");
    Serial.println(sensor.valorActual);
    delay(1000);

    if(sensor.valorActual > DISPARO){
        sensor.status = SECO;
        digitalWrite(valvula.pin, LOW);
        digitalWrite(BOMBA_PIN, LOW);
        valvula.timer = millis();
        while(!timer(valvula.timer, valvula.tiempo)){
        //continue
            delay(1000);
            Serial.println("STATUS>");
            Serial.println("REGANDO");
        }
        digitalWrite(valvula.pin, HIGH);
        digitalWrite(BOMBA_PIN, HIGH);
        Serial.println("STATUS>");
        Serial.println("DEJE DE REGAR");
    }else {
        Serial.print("STATUS>");
        Serial.println("NADA A EJECUTAR");
    }  
}

int lecturaHumedad(){
//toma el valor de humedad    

    sensor.valorActual = analogRead(SENSOR_HUMEDAD_PIN);

    Serial.print("VALOR HUMEDAD> ");
    Serial.println(sensor.valorActual);
    delay(1000);
    return sensor.valorActual;

}

