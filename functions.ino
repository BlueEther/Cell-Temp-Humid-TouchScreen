void printDHT(){
  Serial.print("Current humidity = ");
  Serial.print(DHT.humidity);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(DHT.temperature); 
  Serial.println("C  ");
}

void printMinuteTemps(){
  Serial.print("min counter:  ");
  Serial.println(second()/10);
  Serial.println("sec : T1 : T2");
  for (int i = 0; i < 6; i++){
    Serial.print(i);
    Serial.print(" ");
    Serial.print(minMinMax[0][i],1);
    Serial.print(" / ");
    Serial.println(minMinMax[1][i],1);
  }
}

void printMinMaxAvg(){
  Serial.print("Min Top ");
  Serial.println(minMinMaxArray0.getMin(),1);
  Serial.print("Max Top ");
  Serial.println(minMinMaxArray0.getMax(),1);
  Serial.print("Avg Top ");
  Serial.println(minMinMaxArray0.getAverage(),1);
  Serial.print("Min Bottom ");
  Serial.println(minMinMaxArray1.getMin(),1);
  Serial.print("Max Bottom ");
  Serial.println(minMinMaxArray1.getMax(),1);
  Serial.print("Avg Bottom ");
  Serial.println(minMinMaxArray1.getAverage(),1);
}

void printRunningAvg(){
  hourC ++;
//  Serial.print(F("Running Average: "));
//  Serial.print(hourAvgArray.getAverage(),1);
//  Serial.print(F("deg C. "));
//  Serial.print(hourC);
//  Serial.println(F(" data points."));
}

bool ckError(float temp){
  if(temp == 0) {
    if (SensorFail < 1) { //only publish first error
      if(!HOME){
        warningS.publish("Sensor: Not a valid temp");
        warning.publish(-2);
      }
    }
    SensorFail++;
    Serial.println(F("Sensor: Not a valid temp"));
    return 0;
  } else if(temp == -1) {
    if (SensorFail < 1) { //only publish first error
      if(!HOME){
        warningS.publish("Sensor: Can't contact sensor");
        warning.publish(-2);
      }
    }
    SensorFail++;
    Serial.println(F("Sensor: Can't contact sensor"));
    return 0;
  }
  return 1;
}

void ckAVgTemp(float temp){
  if(DEBUG){
    Serial.print(F("AVG Temp: "));
    Serial.println(temp);
  }
  if(temp > CritTemp){
    if(!HOME){
      warningS.publish("Running: Shit Hot");
      warning.publish(2);
      Serial.println(F("Running: Shit Hot"));
    }
    //digitalWrite(RELAYPin, HIGH);
    myNextion.sendCommand("page1.b4.pco=63488");
    myNextion.sendCommand("page1.b8.pco=63488");
    myNextion.setComponentText("page1.b4","CRITICAL");
    myNextion.setComponentText("page1.b8","V. HOT");
  }else if(temp > OverTemp){
    if(!HOME){
      warningS.publish("Running: Over Temp");
      warning.publish(1);
      Serial.println(F("Running: Over Temp"));
    }    
    myNextion.sendCommand("page1.b4.pco=63488");
    myNextion.sendCommand("page1.b8.pco=63488");
    myNextion.setComponentText("page1.b4","WARNING");
    myNextion.setComponentText("page1.b8","HOT");
  }else if(temp < UnderTemp){
    if(!HOME){
      warningS.publish("Running: Door Open");
      warning.publish(-1);
      Serial.println(F("Running: Door Open"));
    }
    ////digitalWrite(RELAYPin, LOW);
    
    myNextion.sendCommand("page1.b4.pco=63488");
    myNextion.sendCommand("page1.b8.pco=31");
    myNextion.setComponentText("page1.b4","WARNING");
    myNextion.setComponentText("page1.b8","COLD");
  }else {
    if(!HOME){
      warningS.publish("Running: Normal");
      warning.publish(0);
      Serial.println(F("Running: Normal"));
    }
    //digitalWrite(RELAYPin, LOW);
    myNextion.sendCommand("page1.b4.pco=2016");
    myNextion.sendCommand("page1.b8.pco=2016");
    myNextion.setComponentText("page1.b4","Normal");
    myNextion.setComponentText("page1.b8","");
  }
}

void saveTemps(){
  hourAvg[0][minute()]  = minMinMaxArray0.getAverage();
  hourAvg[1][minute()]  = minMinMaxArray1.getAverage();
//  hourMin[minute()]  = minMinMaxArray.getMin();
//  hourMax[minute()]  = minMinMaxArray.getMax();
}

void writeMQTTTemps(){
  //tempHAvg0.publish(hourAvg[0][minute()],1);
  //tempHAvg1.publish(hourAvg[1][minute()],1);
//  tempHMin.publish(hourMin[minute()],1);
//  tempHMax.publish(hourMax[minute()],1);
}

void updateNexData(){
  myNextion.setComponentValue("page1.vTop", tmpOW0*10);
  myNextion.setComponentValue("page1.vTop", tmpOW0*10);
  myNextion.setComponentValue("page1.vMid", tmpOW1*10);
  myNextion.setComponentValue("page1.vHum", humid);
}
void updateNexPage(int page, int button){
  if(page == 1){
    if(button == 2){
      myNextion.sendCommand("click 2,1");   //update screen strings
      myNextion.sendCommand("click 11,1");  //push humidity buttom to diplay graph 
    }else if(button == 3){
      myNextion.sendCommand("click 2,1");   //update screen strings
      myNextion.sendCommand("click 12,1");  //push top temp buttom to diplay graph    
    }else if(button == 5){
      myNextion.sendCommand("click 2,1");   //update screen strings
      myNextion.sendCommand("click 14,1");  //push bottom temp buttom to diplay graph
    }else if(button == 7){
      myNextion.sendCommand("click 2,1");   //update screen strings
      myNextion.sendCommand("click 21,0");  //push / release set temp buttom to diplay graph
      //int32_t value = myNextion.getComponentValue("page1.vTop");
      //Serial.print("Set Temp: ");
      //Serial.println(value);
    }
  }else if(page == 3){
    if(button == 0){
      myNextion.sendCommand("click 4,1");  //push top temp buttom to diplay graph 
    }else if(button == 1){
      myNextion.sendCommand("click 5,1");  //push bottom temp buttom to diplay graph    
    }else if(button == 2){
      myNextion.sendCommand("click 6,1");  //push humidity buttom to diplay graph
    }
  }
}

