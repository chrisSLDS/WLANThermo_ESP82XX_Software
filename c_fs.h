 /*************************************************** 
    Copyright (C) 2016  Steffen Ochs

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    HISTORY: Please refer Github History
    
 ****************************************************/

 // HELP: https://github.com/bblanchon/ArduinoJson

#define CHANNEL_FILE "/channel.json"
#define WIFI_FILE "/wifi.json"
#define THING_FILE "/thing.json"
#define PIT_FILE "/pit.json"
#define SYSTEM_FILE "/system.json"


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Load xxx.json
bool loadfile(const char* filename, File& configFile) {
  
  configFile = SPIFFS.open(filename, "r");
  if (!configFile) {
    
    DPRINTP("[INFO]\tFailed to open: ");
    DPRINTLN(filename);
    
    return false;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    
    DPRINTP("[INFO]\tFile size is too large: ");
    DPRINTLN(filename);
    
    return false;
  }
  return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Save xxx.json
bool savefile(const char* filename, File& configFile) {
  
  configFile = SPIFFS.open(filename, "w");
  
  if (!configFile) {
    
    DPRINTP("[INFO]\tFailed to open file for writing: ");
    DPRINTLN(filename);
    
    return false;
  }  
  return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check JSON
bool checkjson(JsonVariant json, const char* filename) {
  
  if (!json.success()) {
    
    DPRINTP("[INFO]\tFailed to parse: ");
    DPRINTLN(filename);
    
    return false;
  }
  
  return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Load xxx.json at system start
bool loadconfig(byte count) {

  const size_t bufferSize = 6*JSON_ARRAY_SIZE(CHANNELS) + JSON_OBJECT_SIZE(9) + 320;
  DynamicJsonBuffer jsonBuffer(bufferSize);
  File configFile;

  switch (count) {
    
    case 0:     // CHANNEL
    {
      //if (!loadfile(CHANNEL_FILE,configFile)) return false;
      //std::unique_ptr<char[]> buf(new char[configFile.size()]);
      //configFile.readBytes(buf.get(), configFile.size());
      //configFile.close();

      std::unique_ptr<char[]> buf(new char[EECHANNEL]);
      readEE(buf.get(),EECHANNEL, EECHANNELBEGIN);
      
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      if (!checkjson(json,CHANNEL_FILE)) return false;
      
      if (json.containsKey("temp_unit"))  temp_unit = json["temp_unit"].asString();
      else return false;
      
      for (int i=0; i < CHANNELS; i++){
          ch[i].name = json["tname"][i].asString();
          ch[i].typ = json["ttyp"][i];            
          ch[i].min = json["tmin"][i];            
          ch[i].max = json["tmax"][i];                     
          ch[i].alarm = json["talarm"][i];        
          ch[i].color = json["tcolor"][i].asString();        
      }
    }
    break;
    
    case 1:     // WIFI
    {
      std::unique_ptr<char[]> buf(new char[EEWIFI]);
      readEE(buf.get(),EEWIFI, EEWIFIBEGIN);

      JsonArray& _wifi = jsonBuffer.parseArray(buf.get());
      if (!checkjson(_wifi,WIFI_FILE)) return false;
      
      // Wie viele WLAN Schlüssel sind vorhanden
      for (JsonArray::iterator it=_wifi.begin(); it!=_wifi.end(); ++it) {  
        wifissid[lenwifi] = _wifi[lenwifi]["SSID"].asString();
        wifipass[lenwifi] = _wifi[lenwifi]["PASS"].asString();  
        lenwifi++;
      }
    }
    break;
  
    case 2:     // THINGSPEAK
    { 
      std::unique_ptr<char[]> buf(new char[EETHING]);
      readEE(buf.get(),EETHING, EETHINGBEGIN);
      
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      if (!checkjson(json,THING_FILE)) return false;
      
      if (json.containsKey("TSwrite"))  charts.TSwriteKey = json["TSwrite"].asString();
      else return false;
      if (json.containsKey("TShttp"))  charts.TShttpKey = json["TShttp"].asString();
      else return false;
      if (json.containsKey("TSchID"))  charts.TSchID = json["TSchID"].asString();
      else return false;
      if (json.containsKey("TS8"))  charts.TSshow8 = json["TS8"];
      else return false;
    }
    break;

    case 3:     // PITMASTER
    {
      std::unique_ptr<char[]> buf(new char[EEPITMASTER]);
      readEE(buf.get(),EEPITMASTER, EEPITMASTERBEGIN);

      JsonArray& _pid = jsonBuffer.parseArray(buf.get());
      if (!checkjson(_pid,PIT_FILE)) return false;

      pidsize = 0;
      
      // Wie viele Pitmaster sind vorhanden
      for (JsonArray::iterator it=_pid.begin(); it!=_pid.end(); ++it) {  
        pid[pidsize].name = _pid[pidsize]["name"].asString();
        pid[pidsize].id = _pid[pidsize]["id"];
        pid[pidsize].aktor = _pid[pidsize]["aktor"];  
        pid[pidsize].Kp = _pid[pidsize]["Kp"];  
        pid[pidsize].Ki = _pid[pidsize]["Ki"];    
        pid[pidsize].Kd = _pid[pidsize]["Kd"];                     
        pid[pidsize].Kp_a = _pid[pidsize]["Kp_a"];                   
        pid[pidsize].Ki_a = _pid[pidsize]["Ki_a"];                   
        pid[pidsize].Kd_a = _pid[pidsize]["Kd_a"];                   
        pid[pidsize].Ki_min = _pid[pidsize]["Ki_min"];                   
        pid[pidsize].Ki_max = _pid[pidsize]["Ki_max"];                  
        pid[pidsize].pswitch = _pid[pidsize]["switch"];               
        pid[pidsize].reversal = _pid[pidsize]["rev"];                
        pid[pidsize].DCmin    = _pid[pidsize]["DCmin"];              
        pid[pidsize].DCmax    = _pid[pidsize]["DCmax"];              
        pid[pidsize].SVmin    = _pid[pidsize]["SVmin"];             
        pid[pidsize].SVmax    = _pid[pidsize]["SVmax"];   
        pid[pidsize].esum = 0;             
        pid[pidsize].elast = 0;       
        pidsize++;
      }
    }
    if (pidsize < 1) return 0;   // Alte Versionen abfangen
    break;

    case 4:     // SYSTEM
    {
      std::unique_ptr<char[]> buf(new char[EESYSTEM]);
      readEE(buf.get(),EESYSTEM, EESYSTEMBEGIN);
      
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      if (!checkjson(json,SYSTEM_FILE)) return false;
  
      if (json.containsKey("host")) sys.host = json["host"].asString();
      else return false;
      if (json.containsKey("hwalarm")) sys.hwalarm = json["hwalarm"];
      else return false;
      if (json.containsKey("ap")) sys.apname = json["ap"].asString();
      else return false;
      if (json.containsKey("lang")) sys.language = json["lang"].asString();
      else return false;
      if (json.containsKey("utc")) sys.timeZone = json["utc"];
      else return false;
      if (json.containsKey("batmax")) battery.max = json["batmax"];
      else return false;
      if (json.containsKey("batmin")) battery.min = json["batmin"];
      else return false;
      if (json.containsKey("logsec")) {
        int sector = json["logsec"];
        if (sector > log_sector) log_sector = sector;
        // oberes limit wird spaeter abgefragt
      }
      else return false;
      if (json.containsKey("summer")) sys.summer = json["summer"];
      else return false;
      if (json.containsKey("fast")) sys.fastmode = json["fast"];
      else return false;
      if (json.containsKey("hwversion")) sys.hwversion = json["hwversion"];
      else return false;
      if (json.containsKey("update")) sys.update = json["update"];
      else return false;
      if (json.containsKey("autoupd")) sys.autoupdate = json["autoupd"];
      else return false;
      
    }
    break;
    
    //case 5:     // PRESETS
    //break;
  
    default:
    return false;
  
  }

  /*
  DPRINT("[JSON GET]\t");
  json.printTo(Serial);
  DPRINTLN();
  */
  return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Set xxx.json
bool setconfig(byte count, const char* data[2]) {
  
  DynamicJsonBuffer jsonBuffer;
  File configFile;

  switch (count) {
    case 0:         // CHANNEL
    {
      JsonObject& json = jsonBuffer.createObject();

      json["temp_unit"] = temp_unit;

      JsonArray& _name = json.createNestedArray("tname");
      JsonArray& _typ = json.createNestedArray("ttyp");
      JsonArray& _min = json.createNestedArray("tmin");
      JsonArray& _max = json.createNestedArray("tmax");
      JsonArray& _alarm = json.createNestedArray("talarm");
      JsonArray& _color = json.createNestedArray("tcolor");
    
      for (int i=0; i < CHANNELS; i++){
        _name.add(ch[i].name);
        _typ.add(ch[i].typ); 
        _min.add(ch[i].min,1);
        _max.add(ch[i].max,1);
        _alarm.add(ch[i].alarm); 
        _color.add(ch[i].color);
      }

      //if (!savefile(CHANNEL_FILE, configFile)) return false;
      //json.printTo(configFile);
      //configFile.close();
      size_t size = json.measureLength() + 1;
      clearEE(EECHANNEL,EECHANNELBEGIN);  // Bereich reinigen
      static char buffer[EECHANNEL];
      json.printTo(buffer, size);
      writeEE(buffer, size, EECHANNELBEGIN);
    }
    break;

    case 1:        // WIFI
    {
      JsonArray& json = jsonBuffer.createArray();
      clearEE(EEWIFI,EEWIFIBEGIN);  // Bereich reinigen
      static char buffer[3];
      json.printTo(buffer, 3);
      writeEE(buffer, 3, EEWIFIBEGIN);
    }
    break;
    
    case 2:         //THING
    {
      JsonObject& json = jsonBuffer.createObject();
      
      json["TSwrite"] = charts.TSwriteKey;
      json["TShttp"] = charts.TShttpKey;
      json["TSchID"] = charts.TSchID;
      json["TS8"] = charts.TSshow8;
      
      size_t size = json.measureLength() + 1;
      if (size > EETHING) {
        DPRINTPLN("[INFO]\tZu viele THINGSPEAK Daten!");
        return false;
      } else {
        clearEE(EETHING,EETHINGBEGIN);  // Bereich reinigen
        static char buffer[EETHING];
        json.printTo(buffer, size);
        writeEE(buffer, size, EETHINGBEGIN);
      }
    }
    break;

    case 3:        // PITMASTER
    {
      JsonObject& json = jsonBuffer.createObject();
      JsonObject& _master = json.createNestedObject("pm");
      
      _master["ch"]     = pitmaster.channel;
      _master["pid"]    = pitmaster.pid;
      _master["val"]    = pitmaster.value;
      _master["set"]    = pitmaster.set;
      _master["act"]    = pitmaster.active;
      _master["man"]    = pitmaster.manuel;
  
      JsonArray& _pit = json.createNestedArray("pid");
  
      for (int i = 0; i < pidsize; i++) {
        JsonObject& _pid = _pit.createNestedObject();
        _pid["name"]     = pid[i].name;
        _pid["id"]       = pid[i].id;
        _pid["aktor"]    = pid[i].aktor;
        _pid["Kp"]       = pid[i].Kp;  
        _pid["Ki"]       = pid[i].Ki;    
        _pid["Kd"]       = pid[i].Kd;                   
        _pid["Kp_a"]     = pid[i].Kp_a;               
        _pid["Ki_a"]     = pid[i].Ki_a;                  
        _pid["Kd_a"]     = pid[i].Kd_a;             
        _pid["Ki_min"]   = pid[i].Ki_min;             
        _pid["Ki_max"]   = pid[i].Ki_max;             
        _pid["switch"]   = pid[i].pswitch;           
        _pid["rev"]      = pid[i].reversal;                
        _pid["DCmin"]    = pid[i].DCmin;             
        _pid["DCmax"]    = pid[i].DCmax;             
        _pid["SVmin"]    = pid[i].SVmin;             
        _pid["SVmax"]    = pid[i].SVmax;
      }
       
      size_t size = json.measureLength() + 1;
      if (size > EEPITMASTER) {
        DPRINTPLN("[INFO]\tZu viele PITMASTER Daten!");
        return false;
      } else {
        clearEE(EEPITMASTER,EEPITMASTERBEGIN);  // Bereich reinigen
        static char buffer[EEPITMASTER];
        json.printTo(buffer, size);
        writeEE(buffer, size, EEPITMASTERBEGIN);
      }
    }
    break;

    case 4:         // SYSTEM
    {
      String host = HOSTNAME;
      host += String(ESP.getChipId(), HEX);

      JsonObject& json = jsonBuffer.createObject();
  
      json["host"] =        host;
      json["hwalarm"] =     false; 
      json["ap"] =          APNAME;
      json["lang"] =        "de";
      json["utc"] =         1;
      json["summer"] =      false;
      json["fast"] =        false;
      json["hwversion"] =   1;
      json["update"] =      0;
      json["autoupd"] =     1;
      json["batmax"] =      BATTMAX;
      json["batmin"] =      BATTMIN;
      json["logsec"] =      log_sector;
    
      size_t size = json.measureLength() + 1;
      clearEE(EESYSTEM,EESYSTEMBEGIN);  // Bereich reinigen
      static char buffer[EESYSTEM];
      json.printTo(buffer, size);
      writeEE(buffer, size, EESYSTEMBEGIN);
    }
    break;

    case 5:         //PRESETS
    {
      
    }
    break;

    default:
    return false;
  
  }
      
  return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Modify xxx.json
bool modifyconfig(byte count, const char* data[12]) {
  
  DynamicJsonBuffer jsonBuffer;
  File configFile;

  switch (count) {
    case 0:           // CHANNEL
    // nicht notwendig, kann über setconfig beschrieben werden
    break;

    case 1:         // WIFI
    {
      // Alte Daten auslesen
      std::unique_ptr<char[]> buf(new char[EEWIFI]);
      readEE(buf.get(), EEWIFI, EEWIFIBEGIN);

      JsonArray& json = jsonBuffer.parseArray(buf.get());
      if (!checkjson(json,WIFI_FILE)) {
        setconfig(eWIFI,{});
        return false;
      }

      // Neue Daten eintragen
      JsonObject& _wifi = json.createNestedObject();

      _wifi["SSID"] = data[0];
      _wifi["PASS"] = data[1];
      
      // Speichern
      size_t size = json.measureLength() + 1;
      
      if (size > EEWIFI) {
        DPRINTPLN("[INFO]\tZu viele WIFI Daten!");
        return false;
      } else {
        clearEE(EEWIFI,EEWIFIBEGIN);  // Bereich reinigen
        static char buffer[EEWIFI];
        json.printTo(buffer, size);
        writeEE(buffer, size, EEWIFIBEGIN); 
      } 
    }
    break;
    
    case 2:         //THING
    // nicht notwendig, kann über setconfig beschrieben werden
    break;

    case 3:         // PITMASTER
    {
      // Alte Daten auslesen
      std::unique_ptr<char[]> buf(new char[EEPITMASTER]);
      readEE(buf.get(), EEPITMASTER, EEPITMASTERBEGIN);

      JsonArray& json = jsonBuffer.parseArray(buf.get());
      if (!checkjson(json,PIT_FILE)) {
        //set_pid();
        setconfig(ePIT,{});
        return false;
      }

      

      // Neue Daten eintragen
      JsonObject& _pid = json.createNestedObject();
      
      _pid["name"]     = pid[pidsize].name;
      _pid["id"]       = pid[pidsize].id;
      _pid["aktor"]    = pid[pidsize].aktor;
      _pid["Kp"]       = pid[pidsize].Kp;  
      _pid["Ki"]       = pid[pidsize].Ki;    
      _pid["Kd"]       = pid[pidsize].Kd;                   
      _pid["Kp_a"]     = pid[pidsize].Kp_a;               
      _pid["Ki_a"]     = pid[pidsize].Ki_a;                  
      _pid["Kd_a"]     = pid[pidsize].Kd_a;             
      _pid["Ki_min"]   = pid[pidsize].Ki_min;             
      _pid["Ki_max"]   = pid[pidsize].Ki_max;             
      _pid["switch"]   = pid[pidsize].pswitch;           
      _pid["rev"]      = pid[pidsize].reversal;
      _pid["DCmin"]    = pid[pidsize].DCmin;             
      _pid["DCmax"]    = pid[pidsize].DCmax;             
      _pid["SVmin"]    = pid[pidsize].SVmin;             
      _pid["SVmax"]    = pid[pidsize].SVmax;  
    
      // Speichern
      size_t size = json.measureLength() + 1;
      if (size > EEPITMASTER) {
        DPRINTPLN("[INFO]\tZu viele PITMASTER Daten!");
        return false;
      } else {
        clearEE(EEPITMASTER,EEPITMASTERBEGIN);  // Bereich reinigen
        static char buffer[EEPITMASTER];
        json.printTo(buffer, size);
        writeEE(buffer, size, EEPITMASTERBEGIN); 
      }
    }
    break;

    case 4:           // SYSTEM
    {
      // Alte Daten auslesen
      std::unique_ptr<char[]> buf(new char[EESYSTEM]);
      readEE(buf.get(),EESYSTEM, EESYSTEMBEGIN);
      
      JsonObject& alt = jsonBuffer.parseObject(buf.get());
      if (!checkjson(alt,SYSTEM_FILE)) return false;
      
      // Neue Daten erzeugen
      JsonObject& json = jsonBuffer.createObject();

      json["host"] =        sys.host;
      json["hwalarm"] =     sys.hwalarm;
      json["ap"] =          sys.apname;
      json["lang"] =        sys.language;
      json["utc"] =         sys.timeZone;
      json["summer"] =      sys.summer;
      json["fast"] =        sys.fastmode;
      json["hwversion"] =   sys.hwversion;
      json["update"] =      sys.update;
      json["autoupd"] =     sys.autoupdate;
      json["batmax"] =      battery.max;
      json["batmin"] =      battery.min;
      json["logsec"] =      log_sector;

      // Speichern
      size_t size = json.measureLength() + 1;
      clearEE(EESYSTEM,EESYSTEMBEGIN);  // Bereich reinigen
      static char buffer[EESYSTEM];
      json.printTo(buffer, size);
      writeEE(buffer, size, EESYSTEMBEGIN);
      
    }
    break;

    default:
    return false;
  }
  
  return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Initialize FileSystem
void start_fs() {
  
  if (!SPIFFS.begin()) {
    DPRINTPLN("[INFO]\tFailed to mount file system");
    return;
  }

  DPRINTP("[INFO]\tInitalize SPIFFS at Sector: 0x");
  DPRINT((((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE), HEX);
  DPRINTP(" (");
  DPRINT(((uint32_t)&_SPIFFS_end - (uint32_t)&_SPIFFS_start)/1024, DEC);
  DPRINTPLN("K)");
  // 0x40200000 ist der Speicherort des SPI FLASH in der Memory Map

  // CHANNEL
  if (!loadconfig(eCHANNEL)) {
    DPRINTPLN("[INFO]\tFailed to load channel config");
    set_channels(1);
    setconfig(eCHANNEL,{});  // Speicherplatz vorbereiten
    ESP.restart();
  } else DPRINTPLN("[INFO]\tChannel config loaded");


  // WIFI
  if (!loadconfig(eWIFI)) {
    DPRINTPLN("[INFO]\tFailed to load wifi config");
    setconfig(eWIFI,{});  // Speicherplatz vorbereiten
  } else DPRINTPLN("[INFO]\tWifi config loaded");


  // THINGSPEAK
  if (!loadconfig(eTHING)) {
    DPRINTPLN("[INFO]\tFailed to load Thingspeak config");
    setconfig(eTHING,{});  // Speicherplatz vorbereiten
  } else DPRINTPLN("[INFO]\tThingspeak config loaded");


  // PITMASTER
  if (!loadconfig(ePIT)) {
    DPRINTPLN("[INFO]\tFailed to load pitmaster config");
    set_pid();  // Default PID-Settings
    setconfig(ePIT,{});  // Reset pitmaster config
  } else DPRINTPLN("[INFO]\tPitmaster config loaded");


  // SYSTEM
  if (!loadconfig(eSYSTEM)) {
    DPRINTPLN("[INFO]\tFailed to load system config");
    setconfig(eSYSTEM,{});  // Speicherplatz vorbereiten
    ESP.restart();
  } else DPRINTPLN("[INFO]\tSystem config loaded");

}



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Test zum Speichern von Datalog

//unsigned char meinsatz[64] = "Ich nutze ab jetzt den Flash Speicher für meine Daten!\n";
//unsigned char meinflash[64];

void write_flash(uint32_t _sector) {

  noInterrupts();
  if(spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK) {  // ESP.flashEraseSector
    spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, (uint32 *) mylog, sizeof(mylog));  //ESP.flashWrite
    DPRINTP("[LOG]\tSpeicherung im Sector: ");
    DPRINTLN(_sector, HEX);
  } else DPRINTPLN("[INFO]\tFehler beim Speichern im Flash");
  interrupts(); 
}


void read_flash(uint32_t _sector) {

  noInterrupts();
  spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, (uint32 *) archivlog, sizeof(archivlog));  //ESP.flashRead
  //spi_flash_read(_sector * SPI_FLASH_SEC_SIZE, (uint32 *) meinflash, sizeof(meinflash));
  interrupts();
}

void getLog(StreamString *output,int maxlog) {

  //StreamString output;

  int logstart;
  int logend;
  int rest = log_count%MAXLOGCOUNT;
  int saved = (log_count-rest)/MAXLOGCOUNT;    // Anzahl an gespeicherten Sektoren
  
  if (log_count < MAXLOGCOUNT+1) {             // noch alle Daten im Kurzspeicher
    logstart = 0;
    logend = log_count;
  } else {                                    // Daten aus Kurzspeicher und Archiv

    saved = constrain(saved, 0, maxlog);      // maximal angezeigte Logdaten
    int savedend = saved;
    
    if (rest == 0) {                          // noch ein Logpaket im Kurzspeicher
      logstart = 0;                           
      savedend--;
    } else  logstart = MAXLOGCOUNT - rest;   // nur Rest aus Kurzspeicher holen
    
    logend = MAXLOGCOUNT;

    for (int k = 0; k < savedend; k++) {
      Serial.println(log_sector - saved + k,HEX);

      /*
      read_flash(log_sector - saved + k);

      for (int j = 0; j < MAXLOGCOUNT; j++) {
        for (int i=0; i < CHANNELS; i++)  {
          output->print(archivlog[j].tem[i]/10.0);
          output->print(";");
        }
        output->print(archivlog[j].pitmaster);
        output->print(";");
        output->print(digitalClockDisplay(archivlog[j].timestamp));
        output->print("\r\n");
      }
      output->print("\r\n");
      */
    }
  }

  // Kurzspeicher auslesen
  for (int j = logstart; j < logend; j++) {
    for (int i=0; i < CHANNELS; i++)  {
      output->print(mylog[j].tem[i]/10.0);
      output->print(";");
    }
    output->print(mylog[j].pitmaster);
    output->print(";");
    output->print(digitalClockDisplay(mylog[j].timestamp));
    output->print("\r\n");
  }

    /* Test

        read_flash(log_sector-1);
        for (int j=0; j<10; j++) {
          int16_t test = archivlog[j].tem[0];
          Serial.print(test/10.0);
          Serial.print(" ");
        }
    */

   
  //Serial.print(output);
    

    
}

void getLog(File *output,int maxlog) {

  int logstart;
  int logend;
  int rest = log_count%MAXLOGCOUNT;
  int saved = (log_count-rest)/MAXLOGCOUNT;    // Anzahl an gespeicherten Sektoren
  
  if (log_count < MAXLOGCOUNT+1) {             // noch alle Daten im Kurzspeicher
    logstart = 0;
    logend = log_count;
  } else {                                    // Daten aus Kurzspeicher und Archiv

    saved = constrain(saved, 0, maxlog);      // maximal angezeigte Logdaten
    int savedend = saved;
    
    if (rest == 0) {                          // noch ein Logpaket im Kurzspeicher
      logstart = 0;                           
      savedend--;
    } else  logstart = MAXLOGCOUNT - rest;   // nur Rest aus Kurzspeicher holen
    
    logend = MAXLOGCOUNT;

    for (int k = 0; k < savedend; k++) {
      Serial.println(log_sector - saved + k,HEX);

      /*
      read_flash(log_sector - saved + k);

      for (int j = 0; j < MAXLOGCOUNT; j++) {
        for (int i=0; i < CHANNELS; i++)  {
          output->print(archivlog[j].tem[i]/10.0);
          output->print(";");
        }
        output->print(archivlog[j].pitmaster);
        output->print(";");
        output->print(digitalClockDisplay(archivlog[j].timestamp));
        output->print("\r\n");
      }
      output->print("\r\n");
      */
      
    }
  }

  // Kurzspeicher auslesen
  for (int j = logstart; j < logend; j++) {
    for (int i=0; i < CHANNELS; i++)  {
      output->print(mylog[j].tem[i]/10.0);
      output->print(";");
    }
    output->print(mylog[j].pitmaster);
    output->print(";");
    output->print(digitalClockDisplay(mylog[j].timestamp));
    output->print("\r\n");
  }
    
}




