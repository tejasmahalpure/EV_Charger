void connectMQTTBroker() {
  client.setServer(mqtt_broker, mqtt_port);  //connect to mqtt server
  client.setCallback(callback);              //set a callback function, so whenever server sends something that function executes
  int L = 0;
  do{
  if(!client.connected()) {              //stay there until mqtt connection successful
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
    L++;
  }
  }while(L <= 1);

  // Publish and subscribe
  client.publish(topic, "Hi, I'm ESP32 ^^");  //publsh the string to mqtt server
  client.subscribe(topic);                    //subscribe to mqtt server
}

void callback(char* topic, byte* payload, unsigned int length) {  //The mqtt callback function
  
  memset(Data, '\0', sizeof(Data));
  
  for (int i = 0; i < length; i++)  //convert byte array to char array (string)
  {
    Data[i] = (char)payload[i];
  }
  
  Serial.println(Data);
  
  char Token_ID[11],Key[5],cmd[10],Device_Status[15];//, unit[5];
  json_value(Data, "Token_ID", Token_ID);
  json_value(Data, "Key", Key);
  json_value(Data, "cmd", cmd);
  json_value(Data, "unit", unit);


  Serial.println(Flag);
  Serial.println(tokenFlag);
  Serial.println(Token);
  Serial.println(Token_ID);

  if (strcmp(Key, "1") == 0) {
    
    json_edit(Data, "Key", "0");
    Serial.println("New Token:");
    tokenFlag=OPEN;
    strcpy(Token,Token_ID);
    Serial.print(Token_ID);

    if(strcmp(Token,Token_ID)==0 && tokenFlag==OPEN)
    {
      Serial.println("Else if chya aat madhe");

      if (strcmp(cmd, "ON") == 0 && Flag==IDLE)  //if the command received is "ON"
      { 
        Flag = CHARGING;
        json_edit(Data, "status", "Charging_Started");
        charging(1);
        //json_remove(Data,"cmd");
        client.publish(topic, Data);

        Status = "Charging";

        timerStart(timer);

        digitalWrite(chargingProcessInd, HIGH);
        digitalWrite(R, HIGH);
        digitalWrite(G, LOW);
      }

      else if (strcmp(cmd, "OFF") == 0 && Flag==CHARGING)  //if the command received is "OFF"
      {
        Flag = IDLE;
        json_edit(Data, "status", "OFF");
        charging(0);
        
        json_remove(Data,"cmd");

        Status = "IDLE";
        
        calculatePower();
        
        char conunit[5];
        char v[5];
        char i[5];
        
        dtostrf(voltage, 6, 2, v);        0
        json_edit(Data, "Voltage", v);
        dtostrf(Irms, 6, 2, i);
        json_edit(Data, "Current", i);
        dtostrf(consumedPower, 6, 2, conunit);
        json_edit(Data, "unitConsumed", conunit);

        digitalWrite(chargingProcessInd, LOW);
        digitalWrite(G, HIGH);
        digitalWrite(R, LOW);
        
        client.publish(topic, Data);
        timerRestart(timer);
        timerStop(timer);
      }

      else if  (strcmp(cmd, "GetStat") == 0){  
        Serial.println("Device Status");
        json_remove(Data, "cmd");
        json_add(Data,sizeof(Data),"status",Status);
        calculatePower();
        char conunit[5];
        char v[5];
        char i[5];
        
        dtostrf(voltage, 6, 2, v);        
        json_edit(Data, "Voltage", v);
        dtostrf(Irms, 6, 2, i);
        json_edit(Data, "Current", i);
        dtostrf(consumedPower, 6, 2, conunit);
        json_edit(Data, "unitConsumed", conunit);
                
        client.publish(topic, Data);
      }

      else if (strcmp(cmd, "download") == 0){
        if(Flag == IDLE){
          checkForUpdate();  
        }
        else{
          client.publish(topic, "Charging in Progress, update LATER");
        }
      }

      else{
        Serial.println("Wrong Command !!!!!!!!");
      }
    }


  } else {
    Serial.println("Callback Echo");
  }
}

void charging(bool cmd) {
  bool comd = cmd;
  if (comd == 1) {
    digitalWrite(4, HIGH);
  }

  else if (comd == 0) {
    digitalWrite(4, LOW);
  }
}

int json_remove(char* json, char* key) {
  //code to extract the desired "val" at "key"
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return 0;
  }

  doc.remove(key);

  serializeJson(doc, json, 200);
  return 0;
}

int json_value(char* json, char* key, char* val) {
  //code to extract the desired "val" at "key"
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return 0;
  }

  const char* value = doc[key];
  // Serial.print(value);
  if (value) {
    strcpy(val, value);
    return 0;
  }

  return 0;
}

int json_edit(char* json, char* key, char* val) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return 0;
  }

  doc[key] = val;

  serializeJson(doc, json, 200);
  return 0;
}

int json_add(char* json, size_t jsonSize, const char* key, const char* value) {
  // Create a JSON document with enough capacity
  StaticJsonDocument<512> doc;

  // Deserialize the JSON string into the document
  DeserializationError error = deserializeJson(doc, json);

  // Check if deserialization was successful
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return -1;  // Indicate deserialization failure
  }

  // Add the new key-value pair
  doc[key] = value;

  // Measure the size of the modified JSON document
  size_t newSize = measureJson(doc) + 1;  // +1 for null terminator

  // Check if the new size exceeds the buffer size
  if (newSize > jsonSize) {
    Serial.println(F("Buffer size is too small for the modified JSON."));
    return -2;  // Indicate buffer size too small
  }

  // Serialize the modified JSON document back to the input JSON string
  serializeJson(doc, json, jsonSize);
  return 1;  // Indicate success
}
