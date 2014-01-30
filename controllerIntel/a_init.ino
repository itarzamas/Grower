/********************************************
  Initialization Functions
********************************************/

void initSd(){
  
  //initialize SD
  #if defined(DEBUG) && defined(USESERIALMONITOR)  
      Serial.print("|| SD ...");
  #endif
  
   pinMode(hardwareSelect, OUTPUT);
  //pinMode(10, OUTPUT);

  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    #if defined(DEBUG) && defined(USESERIALMONITOR)        
      Serial.println("FAIL (unable to init)");
    #endif
  } else {
    #if defined(DEBUG) && defined(USESERIALMONITOR)
      Serial.print("OK");
      Serial.print("card information:");
      Serial.print("||  type=");
      switch(card.type()) {
        case SD_CARD_TYPE_SD1:
          Serial.println("SD1");
          break;
        case SD_CARD_TYPE_SD2:
          Serial.println("SD2");
          break;
        case SD_CARD_TYPE_SDHC:
          Serial.println("SDHC");
          break;
        default:
          Serial.println("Unknown");
      }
      Serial.print("||  volume=");
    #endif
    if (!volume.init(card)) {
    }else{
      // print the type and size of the first FAT-type volume
      root.openRoot(volume);
      #if defined(DEBUG) && defined(USESERIALMONITOR)  
        uint32_t volumesize;
        Serial.print("FAT ");
        Serial.println(volume.fatType(), DEC);
        volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
        volumesize *= volume.clusterCount();       // we'll have a lot of clusters
        volumesize *= 512;                            // SD card blocks are always 512 bytes
        volumesize /= 1024;
        volumesize /= 1024;
        Serial.print("||  size(MB) = ");
        Serial.println(volumesize);
        Serial.println("||  Files: ");
        // list all files in the card with date and size
        root.ls(LS_R | LS_DATE | LS_SIZE);
      #endif
      
      SD.begin(chipSelect);
    }
    
  }
  
}

void initRtc(){

  //initialize clock
  #if defined(DEBUG) && defined(USESERIALMONITOR)
    Serial.print("|| RTC...");
  #endif
  
  Wire.begin();
  //RTC.begin();
    
  //if not available, nothing to do
  /*if(!RTC.isrunning()){
    #if defined(DEBUG)
      Serial.println("NOT AVAILABLE");
    #endif
  }else{*/
    
    //RTC is available, lets go ahead and indicate that the time is synced
    //timeSynced = true;
    //timeSyncedDateTime =  getLocalTime();
    
      #if defined(SETTIME)
          #if defined(DEBUG) && defined(USESERIALMONITOR)  
            Serial.print("Manually setting to compile time...");
          #endif
          // following line sets the RTC to the date & time this sketch was compiled
          //RTC.adjust(DateTime(__DATE__, __TIME__));
      #endif
    
    #if defined(DEBUG) && defined(USESERIALMONITOR)  
      Serial.print("OK");
      Serial.print("(");
      printDateTimeToSerial(timeSyncedDateTime);
      Serial.print(")");
      Serial.println();
    #endif
  //}
   
}

//fix-me: better place for this?
DateTime getLocalTime(){
  
    //if time hasn't been synced yet, than there is nothing to do.
    if(timeSynced==false){
      #if defined(DEBUG)
        Serial.print("error: time hasn't been synced yet");
        stringErrorSync.print(Serial);
        Serial.println("");
      #endif
    }else{
    
      //otherwise, if RTC isn't running use the synced time
      //if(!RTC.isrunning()){
        
          int secondsSinceSync = (millis()/1000) - timeAtSync;
          
          //resync every 5 minutes
          if(timeSyncInProgress==false && secondsSinceSync>(60*5)){
            //and initiate sync again
            sendCommand("c:time>");
          }
          
          //get the last synced time
          return DateTime(timeSyncedDateTime.unixtime()+secondsSinceSync);
          
      //or get the synced time directly from the RTC
      //}else{
        //return DateTime(RTC.now().unixtime() + (config.utcOffset*60*60));
        //return DateTime(RTC.now().unixtime());
      //}

    }
  
}


void initController(){
 
  initSensors();
  initZones();
  initSchedules();

}

void initSensors(){
  #if defined(DEBUG)
    Serial.println("|| Initializing sensors:");
  #endif
  for(int i=0;i<maxSensors;i++){
    #if defined(DEBUG)
      Serial.print("||    ");
      Serial.print(i);
      Serial.print(" - ");
    #endif
    initSensor(configStore.sensors[i]);
  }
}

void initSensor(struct Sensor &thisSensor){
 
  if(thisSensor.type==0){
    //zone off
    #if defined(DEBUG)
      Serial.print("Disabled");
    #endif
  }else if(thisSensor.type==1){
    //soil moisture (analog)
    pinMode(thisSensor.pin, INPUT);
    #if defined(DEBUG)
      Serial.print("Analog");
    #endif
  }else if(thisSensor.type==2){
    //DS18B20
    pinMode(thisSensor.pin, INPUT);
    #if defined(DEBUG)
      Serial.print("DS18B20");
    #endif
  }else if(thisSensor.type==3){
    //dht22
    pinMode(thisSensor.pin, INPUT);
    #if defined(DEBUG)
      Serial.print("DHT22");
    #endif
  }else if(thisSensor.type==4){
    //light
    pinMode(thisSensor.pin, INPUT);
    #if defined(DEBUG)
      Serial.print("Light");
    #endif
  }
  
  #if defined(DEBUG)
    Serial.print(" - ");
    Serial.print(thisSensor.name);
    
    if(thisSensor.type>0){
      Serial.print(" (");
      Serial.print("pin=");
      Serial.print(thisSensor.pin);
      Serial.print(", ");
      Serial.print("pin2=");
      Serial.print(thisSensor.pin2);
      Serial.print(", ");
      Serial.print("frequencyCheckSeconds=");
      Serial.print(thisSensor.frequencyCheckSeconds);
      Serial.print(", ");
      Serial.print("frequencyLogSeconds=");
      Serial.print(thisSensor.frequencyLogSeconds);
      Serial.print(")");
    }
    
    Serial.println();
  #endif

}

void initZones(){
  #if defined(DEBUG) && defined(USESERIALMONITOR)  
    Serial.println("|| Initializing zones:");
  #endif
  for(int i=0;i<maxZones;i++){
    #if defined(DEBUG) && defined(USESERIALMONITOR)  
      Serial.print("||    ");
      Serial.print(i);
      Serial.print(" - ");
    #endif
    initZone(configStore.zones[i]);
  }
}

void initZone(struct Zone &thisZone){

  if(thisZone.type==0){
    //sensor off
    #if defined(DEBUG)
      Serial.print("Disabled");
    #endif
  }else if(thisZone.type==1){
    #if defined(DEBUG)
      Serial.print("5V Relay");
    #endif
    pinMode(thisZone.pin, OUTPUT);
  }
 
  #if defined(DEBUG)
    Serial.print(" - ");
    Serial.print(thisZone.name);
    Serial.println();
  #endif
  
}

void initSchedules(){
  #if defined(DEBUG)
    Serial.println("|| Initializing schedules:");
  #endif
  for(int i=0;i<maxSchedules;i++){
    #if defined(DEBUG)
      Serial.print("||    ");
      Serial.print(i);
      Serial.print(" - ");
    #endif
    initSchedule(configStore.schedules[i]);
  }
}

void initSchedule (struct Schedule &thisSchedule){
    
  if(thisSchedule.type==0){
    //sensor off
    #if defined(DEBUG)
      Serial.print("Disabled");
    #endif
  }else if(thisSchedule.type==1){
    #if defined(DEBUG)
      Serial.print("Timer");
    #endif
  }else if(thisSchedule.type==2){
    #if defined(DEBUG)
      Serial.print("Soil Moisture");
    #endif
  }else if(thisSchedule.type==3){
    #if defined(DEBUG)
      Serial.print("Temperature");
    #endif
  }
  
  //0=off, 1=timer, 2=soil moisture, 3=temperature
  
  #if defined(DEBUG)
    if(thisSchedule.type!=0){
      Serial.print(" - ");
      Serial.print(thisSchedule.name);
      Serial.print(" (sensors=");
      for(int i=0;i<maxSensors;i++){
        int thisSensorId = thisSchedule.sensors[i];
        if(thisSensorId > -1){
          //we subtract one because this is the actual ID
          Serial.print(configStore.sensors[thisSensorId].name);
          Serial.print(",");
        }
      }
        
      Serial.print(" zones=");
      for(int i=0;i<maxZones;i++){
        int thisZoneId = thisSchedule.zones[i];
        //Serial.println(thisZoneId);
        if(thisZoneId > -1){
          //we subtract one because this is the actual ID
          Serial.print(configStore.zones[thisZoneId].name);
          Serial.print(",");
        }
      }
      
      Serial.print(")");
    
    }
    Serial.println();
  #endif
  
}
