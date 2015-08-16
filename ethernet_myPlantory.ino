/*
ethernet_myPlantory.ino

author: Eden Candelas
@elmundoverdees
HackerSpace Monterrey

Contiene las funciones de red myPlantory.
Sigue el esquema de arduino, solo contiene las declaraciones del
las funciones.


*/

void enviarRequest(EthernetClient client, struct valores *lecturas){
/*
Esta funcion es la que periodicamente esta enviando los valores de los sensores al servidor
el request enviado esta en la forma : "GET /valores?l=1024&t=100.00&h=1024 HTTP/1.1"

Los valores dentro de serverAddress identifican la ip a la que se estaran enviando los datos.

*/

  IPAddress serverAddress(192,168,1,100);  
  int serverPort = 8081;

  if (client.connect(serverAddress, serverPort)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    // client.println("GET /?l=lecturasLuminosidad&t=lecturaTemperatura&h=lecturaTemperatura HTTP/1.1");
    client.print("GET /valores?l=" );
    client.print(lecturas->valorLuminosidad);
    client.print("&t=");
    client.print(lecturas->valorTemperatura);
    client.print("&h=");
    client.print(lecturas->valorHumedad);
    client.println(" HTTP/1.1");
    client.println("Host: www.arduino.cc");
    client.println("User-Agent: arduino-ethernet");
    client.println("Connection: close");
    client.println();

    Serial.println("Data sent");

  } 
  else {
    Serial.println("connection failed");
    Serial.println("disconnecting.");
    client.stop();
  }
}

void procesarCliente(EthernetClient client, struct valores *lecturas){
/*
Esta funcion es llamada cuando ocurre una conexion hacia el arduino, la estructura lecturas
es tiene para manejar en un solo objeto las tres lecturas que necesitamos

Esta funcion despliega el html basico con el que el arduino responde.

Una seccion con los valores de las lecturas y llama a procesarSubmit()
que es la funcion que se encarga de crear el html de los botones y ejecutar
riego o lampara si los botones fueron presionados.

Desconecta la sesion al finalizar.

*/

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
/*
    se crean dos botones que hacen que el navegador envie un GET de vuelta al arduino
    con los valores "regar=on" o "lampara=on".
    Si se encuentra que la peticion que entra tiene alguno de estos valores
    esta presente llama a su respectiva funcion ejecutarLampara o ejecutarRiego (el codigo
    de estas funciones esta en sensores_myPlantory.ino)
*/


    cl.println("<form action=\"action_page.php\" method=\"GET\">");
    cl.println("<input type=\"submit\" name=\"regar\"value=\"on\">");
    cl.println("<form/>");

    cl.println("<form action=\"action_page.php\" method=\"GET\">");
    cl.println("<input type=\"submit\" name=\"lampara\"value=\"on\">");
    cl.println("<form/>");

    int indexLampara = HTTP_req.indexOf("lampara=on");
    Serial.print("indexLampara: ");
    Serial.println(indexLampara);

    if (indexLampara > -1 && indexLampara < 50){
        // Serial.println("DEBE REGAR");
        ejecutarLampara();
    }


    int indexRegar = HTTP_req.indexOf("regar=on");
    Serial.print("indexRegar: ");
    Serial.println(indexRegar);


    if (indexRegar > -1 && indexRegar < 50){
        // Serial.println("DEBE REGAR");
        ejecutarRiego();
    }
}

void configureEthernet(){
/* Inicializamos el modulo ethernet con una ip statica, se puede remover ip para que se
se realize dhcp
*/  
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("ETHERNET> server is at");
  Serial.println(Ethernet.localIP());


}