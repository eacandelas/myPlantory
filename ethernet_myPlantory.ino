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
  // if there's a successful connection:
  // char serverAddress[] = "www.arduino.cc";  
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
// start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("ETHERNET> server is at");
  Serial.println(Ethernet.localIP());


}