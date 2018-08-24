// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 1;
  int i = 0;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.print("Retrying MQTT connection in ");
       Serial.print(10 * i);
       Serial.println(" seconds...");
       mqtt.disconnect();
       delay(10000 * i);  // wait 10 x i value seconds
       i++;
       if (retries == i) {
         Serial.println("MQTT NOT Connected!");
         // basically die and wait for WDT to reset me
         //while (1);
         return;
       }
  }
  Serial.println("MQTT Connected!");
}

