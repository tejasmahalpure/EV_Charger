void getDeviceID() {
  if (WiFi.status() == WL_CONNECTED) {  // Check if Wi-Fi is connected
    HTTPClient http;
    http.begin(host);
    http.addHeader("Content-Type", "application/json");  // Set content type to JSON

    // Create JSON document
    String macAddress = WiFi.macAddress();  //"AC:12:03:64:BB:7B";
    String JsonDoc = "{\"MAC_ID\":\"" + macAddress + "\"}";

    // Send HTTP POST request
    int httpResponseCode = http.POST(JsonDoc);

    // Print request body
    Serial.println("Sending: ");
    Serial.println(JsonDoc);

    // Read response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);

      // Parse JSON response
      DynamicJsonDocument jsonResponse(1024);
      DeserializationError error = deserializeJson(jsonResponse, response);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      } else {
        // Extract data from JSON response and assign to variables
        String receivedDeviceID = jsonResponse["Device_ID"];
        rxDevID = receivedDeviceID;
        Serial.print("Device ID: ");
        Serial.println(receivedDeviceID);
      }
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();  // Free resources
  } else {
    Serial.println("WiFi Disconnected");
  }
}
