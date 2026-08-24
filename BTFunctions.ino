void BTFunctions() {
  if (serialBT.available()) {
    cmd = serialBT.read();

    if (cmd == '1') {
    serialBT.print("MAC ID of device: ");
    String baseMAC = WiFi.macAddress();
    serialBT.print(baseMAC);

    serialBT.println("");
    serialBT.print("Device ID is: ");
    serialBT.print(rxDevID);

    serialBT.println("");
    }

    else if (cmd == '2') {
      serialBT.println("Current network ssid is: ");
      serialBT.println(SSIDbuffer);
    }

    else if (cmd == '3') {
      serialBT.println("Current network Password");
      serialBT.println(PassBuffer);
    }

    else if (cmd == '4') {
      //char newSSID[16];
      //Serial.println("Char create jhalay");
      if (serialBT.available()) {
        char temp = serialBT.read();
        if (temp == '<') {
          //Serial.println("1");
          //Serial.println(temp);
          int i = 0;
          memset(newSSID, '\0', sizeof(newSSID));
          while (1) {
            temp = serialBT.read();
            if (temp == '>') {
              break;
            }
            //Serial.println("2");
            //Serial.println(temp);
            newSSID[i] = temp;
            i++;
          }
          //Serial.println("3");
          //Serial.println(temp);
          Serial.print("received:");
          Serial.println(newSSID);
        }
      }
    }

    else if (cmd == '5') {
      //char newPass[16];
      if (serialBT.available()) {
        char temp = serialBT.read();
        if (temp == '<') {
          int i = 0;
          memset(newPass, '\0', sizeof(newPass));
          while (1) {
            temp = serialBT.read();
            if (temp == '>') {
              break;
            }
            newPass[i] = temp;
            i++;
          }
          Serial.print("received:");
          Serial.println(newPass);
        }
      }
    }

    else if (cmd == '6') {  //remove this and when BT is OFF check weather ssid and password has been changed and then connect to new wifi.
      // Serial.println(newSSID);
      // Serial.println(newPass);

      // int j;
      // for(j = 0; newSSID[j] != '\0'; j++){
      // }
      // Serial.println(j);

      // WiFi.begin(newSSID, newPass);
      // while (WiFi.status() != WL_CONNECTED) {
      // Serial.print(".");
      // delay(500);
      // }
      // Serial.println();
      // Serial.print("Connected to: ");
      // Serial.println(newSSID);

      pref.begin("my-app", false);
      pref.putString("SSID", newSSID);
      pref.putString("PassWord", newPass);
      Serial.println("SSID and Password written to memory");
      pref.end();
      
      Serial.println(newSSID);
      Serial.println(newPass);
      WiFi.begin(newSSID, newPass);
      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
      }
      connectMQTTBroker();
      digitalWrite(networkStatusInd, HIGH);
    }
  }
}
