 void calculatePower() {
   voltage = voltageSensor.getRmsVoltage();
   Serial.print("Voltage: ");
   Serial.println(voltage);
   float current; 
   sum = 0.0;
  
   for(int i = 0;i < 500; i++){
   int Iadc_val = analogRead(ACS_ADC);
  
   float sig_voltage = ((Iadc_val * 3.3 )/ 4095.0) + 0.07;
  
   if (sig_voltage - offset > 0) {
     current = (sig_voltage-offset)/sensitivity;
   }
  
   if (sig_voltage - offset < 0) {
     current = -(sig_voltage-offset)/sensitivity;
   }

   if (current < 0) {
     current = 0;
}
  
   sum += current;
   delay(1);
   }
     float avg = sum/1000;
    
   Irms = sqrt(avg);  
   Serial.println(Irms);
   instantPower = voltage*Irms;
   double t = timerReadSeconds(timer);
   float tinHr = t / 3600;
   consumedPower = (instantPower * tinHr)/1000;
   Serial.println(consumedPower);
   Serial.println(t);
 }

void Indicators(){
  
}
