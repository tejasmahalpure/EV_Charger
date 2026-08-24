void checkForUpdate() {
  Serial.println("Checking for firmware update...");

  HTTPClient http;
  http.begin(firmwareURL);
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    int contentLength = http.getSize();
    bool canBegin = Update.begin(contentLength);

    if (canBegin) {
      Serial.println("Starting update...");
      WiFiClient *client = http.getStreamPtr();
      size_t written = Update.writeStream(*client);

      if (written == contentLength) {
        Serial.println("Written: " + String(written) + " bytes");
      } else {
        Serial.println("Failed to write complete file");
      }

      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("Update successful, restarting...");
          ESP.restart();
        } else {
          Serial.println("Update not finished, something went wrong.");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      Serial.println("Not enough space to begin OTA update.");
    }
  } else {
    Serial.println("Failed to check for update, HTTP code: " + String(httpCode));
  }
  http.end();
}
