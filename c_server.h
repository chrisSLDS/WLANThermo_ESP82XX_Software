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


// Beispiele:
// https://github.com/spacehuhn/wifi_ducky/blob/master/esp8266_wifi_duck/esp8266_wifi_duck.ino

struct myRequest {
  String host;
  String url;
  String method;
  String response;
  AsyncWebServerRequest *request;
};

myRequest myrequest;

static AsyncClient * aClient = NULL;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check if there is http update
void getRequest() {

  if(aClient) return;                 //client already exists

  aClient = new AsyncClient();
  if(!aClient)  return;               //could not allocate client

  aClient->onError([](void * arg, AsyncClient * client, int error){
    aClient = NULL;
    delete client;
  }, NULL);

  aClient->onConnect([](void * arg, AsyncClient * client){

   aClient->onError(NULL, NULL);

   client->onDisconnect([](void * arg, AsyncClient * c){
    DPRINTPLN("[INFO]\tDisconnect myRequest Client");
    myrequest.request->send(200, "text/plain", myrequest.response);
    myrequest.response = "";
    aClient = NULL;
    delete c;
   }, NULL);

   client->onData([](void * arg, AsyncClient * c, void * data, size_t len){
    String payload((char*)data);
    myrequest.response += payload; 
    Serial.println("[INFO]\tResponse external Request");
   }, NULL);

   //send the request
   DPRINTPLN("[INFO]\tSend external Request:");
   String url;
   if (myrequest.method == "POST") url += F("POST ");
   else  url += F("GET ");
   url += myrequest.url;
   url += F(" HTTP/1.1\n");
   url += F("Host: ");
   url += myrequest.host;
   Serial.println(url);
   url += F("\n\n");

   client->write(url.c_str());
    
 }, NULL);

 if(!aClient->connect(myrequest.host.c_str(), 80)){
   Serial.println("[INFO]\MyRequest Client Connect Fail");
   AsyncClient * client = aClient;
   aClient = NULL;
   delete client;
 }    
}



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
String serverLog() {

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["SN"] = String(ESP.getChipId(), HEX);
  JsonArray& _logs = root.createNestedArray("logs");

  if (log_count > 9) {
    
    for (int i = 0; i < 10; i++) {
      if (mylog[i].modification) {    // nur bei Aenderung wird das Log benutzt
        JsonObject& _log = _logs.createNestedObject();
        _log["time"] = mylog[i].timestamp;
        _log["battery"] = mylog[i].battery;
        _log["pit_set"] = (mylog[i].soll==NULL)?NULL:mylog[i].soll/10.0;
        _log["pit_value"] = mylog[i].pitmaster; 
        _log["ch1"] = (mylog[i].tem[0]==NULL)?NULL:mylog[i].tem[0]/10.0;
        _log["ch2"] = (mylog[i].tem[1]==NULL)?NULL:mylog[i].tem[1]/10.0;
        _log["ch3"] = (mylog[i].tem[2]==NULL)?NULL:mylog[i].tem[2]/10.0;
        _log["ch4"] = (mylog[i].tem[3]==NULL)?NULL:mylog[i].tem[3]/10.0;
        _log["ch5"] = (mylog[i].tem[4]==NULL)?NULL:mylog[i].tem[4]/10.0;
        _log["ch6"] = (mylog[i].tem[5]==NULL)?NULL:mylog[i].tem[5]/10.0;
        _log["ch7"] = (mylog[i].tem[6]==NULL)?NULL:mylog[i].tem[6]/10.0;
        _log["ch8"] = (mylog[i].tem[7]==NULL)?NULL:mylog[i].tem[7]/10.0;
      } 
    }
  }
 
  String json;
  root.printTo(json);
  
 /*
 
  String json = "{\"SN\":\"";
  json += String(ESP.getChipId(), HEX);
  json += "\",\"logs\":[";
  
  if (log_count > 9) {
    
    for (int i = 0; i < 10; i++) {
      if (mylog[i].modification) {    // nur bei Aenderung wird das Log benutzt
        if (i > 0) json += ",";
        json += "{\"time\":";
        json += mylog[i].timestamp;
        json += ",\"battery\":";
        json += mylog[i].battery;
        json += ",\"pit_set\":";
        if (mylog[i].soll=!NULL) json += mylog[i].soll/10.0;
        json += ",\"pit_value\":";
        json += mylog[i].pitmaster; 
        for (int j = 0; j < 8; j++) {
          json += ",\"ch";
          json += String(j);
          json += "\":";
          if (mylog[i].tem[j]!=NULL)  json += mylog[i].tem[j]/10.0;
        }
        json += "}";
      } 
    }
  }
  json += "]}";

  */
  return json;
}


static AsyncClient * LogClient = NULL;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
void sendServerLog() {

  if(LogClient) return;                 //client already exists

  LogClient = new AsyncClient();
  if(!LogClient)  return;               //could not allocate client

  LogClient->onError([](void * arg, AsyncClient * client, int error){
    LogClient = NULL;
    delete client;
  }, NULL);

  LogClient->onConnect([](void * arg, AsyncClient * client){

   LogClient->onError(NULL, NULL);

   client->onDisconnect([](void * arg, AsyncClient * c){
    DPRINTPLN("[INFO]\tDisconnect Log Client");
    LogClient = NULL;
    delete c;
   }, NULL);

   client->onData([](void * arg, AsyncClient * c, void * data, size_t len){
    String payload((char*)data);
    serverAnswer(payload, len);
   }, NULL);

   //send the request
   DPRINTPLN("[INFO]\tSend Log to Server:");

   String message = serverLog(); 
   String adress = createCommand(POSTMETH,NOPARA,SAVELOGSLINK,NANOSERVER,message.length());
   adress += message;
   client->write(adress.c_str());
   //Serial.println(adress);
      
 }, NULL);

 if(!LogClient->connect("nano.wlanthermo.de", 80)){
   Serial.println("[INFO]\Log Client Connect Fail");
   AsyncClient * client = LogClient;
   LogClient = NULL;
   delete client;
 }    
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
String cloudData() {

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& system = root.createNestedObject("system");

    system["time"] = String(now());
    system["utc"] = sys.timeZone;
    system["soc"] = battery.percentage;
    system["charge"] = !battery.charge;
    system["rssi"] = rssi;
    system["unit"] = temp_unit;
    //system["sn"] = String(ESP.getChipId(), HEX);

    JsonArray& channel = root.createNestedArray("channel");

    for (int i = 0; i < CHANNELS; i++) {
    JsonObject& data = channel.createNestedObject();
      data["number"]= i+1;
      data["name"]  = ch[i].name;
      data["typ"]   = ch[i].typ;
      data["temp"]  = limit_float(ch[i].temp, i);
      data["min"]   = ch[i].min;
      data["max"]   = ch[i].max;
      data["alarm"] = ch[i].alarm;
      data["color"] = ch[i].color;
    }
  
    JsonObject& master = root.createNestedObject("pitmaster");

    master["channel"] = pitmaster.channel+1;
    master["pid"] = pitmaster.pid;
    master["value"] = (int)pitmaster.value;
    master["set"] = pitmaster.set;
    if (pitmaster.active)
      if (autotune.initialized)  master["typ"] = "autotune";
      else if (pitmaster.manual) master["typ"] = "manual";
      else  master["typ"] = "auto";
    else master["typ"] = "off";  

    String jsonStr;
    root.printTo(jsonStr);
  
    return jsonStr;
  
}


static AsyncClient * DataClient = NULL;

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
void sendDataCloud() {

  if(DataClient) return;                 //client already exists

  DataClient = new AsyncClient();
  if(!DataClient)  return;               //could not allocate client

  DataClient->onError([](void * arg, AsyncClient * client, int error){
    LogClient = NULL;
    delete client;
  }, NULL);

  DataClient->onConnect([](void * arg, AsyncClient * client){

   DataClient->onError(NULL, NULL);

   client->onDisconnect([](void * arg, AsyncClient * c){
    DPRINTPLN("[INFO]\tDisconnect Data Cloud Client");
    DataClient = NULL;
    delete c;
   }, NULL);

   client->onData([](void * arg, AsyncClient * c, void * data, size_t len){
    String payload((char*)data);
    serverAnswer(payload, len);
   }, NULL);

   //send the request
   DPRINTPLN("[INFO]\tSend Data to Cloud:");
   String message = cloudData();   
   String adress = createCommand(GETMETH,SAVEDATA,SAVEDATALINK,NANOSERVER,message.length());
   adress += message;
   client->write(adress.c_str());
   //Serial.println(adress);
      
  }, NULL);

  if(!DataClient->connect("nano.wlanthermo.de", 80)){
   Serial.println("[INFO]\Data Client Connect Fail");
   AsyncClient * client = DataClient;
   DataClient = NULL;
   delete client;
  }    
}





// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
/*
int dothis;
int histthis;
int saved;
int savedstart;
int savedend;
int logstart;

void handlePlot(AsyncWebServerRequest *request, bool www) {

    unsigned long vorher = millis(); 
    int maxlog = 4;
    //int rest = log_count%MAXLOGCOUNT;
    int rest = 0;
    
    saved = (log_count-rest)/MAXLOGCOUNT;    // Anzahl an gespeicherten Sektoren
    saved = constrain(saved, 0, maxlog);
    
    savedstart = 0;
  
    if (log_count < MAXLOGCOUNT) {             // noch alle Daten im Kurzspeicher
      savedend = 0;
      logstart = 0;
      dothis = log_count;
    } else {                                    // Daten aus Kurzspeicher und Archiv
      savedend = saved;
      histthis = MAXLOGCOUNT;
      if (rest == 0) {                          // noch ein Logpaket im Kurzspeicher
        dothis = MAXLOGCOUNT; 
        logstart = 0;                          
        savedend--;
      } else { 
        dothis = rest;   // nur Rest aus Kurzspeicher holen
        logstart = MAXLOGCOUNT - rest;
      }
    }  
    
    AsyncWebServerResponse *response = request->beginChunkedResponse("text/html", [vorher](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
    
      StreamString output;
      Serial.println(maxLen);
    
      if (index == 0) {
        Serial.print("first: ");
        output.print(F("<html>\n<head>\n<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>\n"));
        output.print(F("<script type=\"text/javascript\">\n google.charts.load('current', {packages: ['corechart', 'line']});\ngoogle.charts.setOnLoadCallback(drawChart);\n"));
        output.print(F("function drawChart() {\nvar data = new google.visualization.DataTable();\ndata.addColumn('datetime','name');\ndata.addColumn('number','Pitmaster');\ndata.addColumn('number','Soll');\n"));
        output.print(F("data.addColumn('number','Kanal1');\n"));
        output.print(F("data.addColumn('number','Kanal2');\n"));
        output.print(F("data.addColumn('number','Kanal7');\n"));
        output.print(F("data.addRows([\n"));

        Serial.println(output.length());
      
        if (output.length() < maxLen) {
          output.getBytes(buffer, maxLen); 
          return output.length();
        } else return 0;

      //} else if (savedstart < savedend) {
//*
        read_flash(log_sector - saved + savedstart);
        
        while (output.length()+50 < maxLen && histthis != 0) {

          int j = MAXLOGCOUNT - histthis;
          
          output.print(F("["));
          output.print(newDate(archivlog[j].timestamp));
          output.print(F(","));
          output.print(archivlog[j].pitmaster);
          output.print(",");
          output.print(archivlog[j].soll);

          for (int i=0; i < 1; i++)  {
            output.print(",");
            if (archivlog[j].tem[i]/10 == INACTIVEVALUE)
              output.print(0);
            else
              output.print(archivlog[j].tem[i]/10.0);   
          } 
          output.print("],\n");

          histthis--;
        
        }
          
        Serial.print("next_");
        Serial.print(log_sector - saved + savedstart,HEX);
        Serial.print(": ");
        Serial.println(output.length());
        
        if (histthis == 0) {
          savedstart++;
          if (savedstart < savedend) histthis = MAXLOGCOUNT;
        }
      
        output.getBytes(buffer, maxLen); 
        return output.length();
 //
      
      } else if (dothis > 0)  {


        while (output.length()+50 < maxLen && dothis != 0) {  
        
          int j;
          if (log_count<MAXLOGCOUNT) j = log_count - dothis;
          else j = MAXLOGCOUNT - dothis;     // ACHTUNG: log_count verändert sich, könnte blöd werden
        
          //Serial.print(log_count); Serial.print(" | ");
          //Serial.print(log_count%MAXLOGCOUNT); Serial.print(" | ");
          //Serial.println(dothis);
        
          output.print(F("["));
          output.print(newDate(mylog[j].timestamp));
          output.print(F(","));
          output.print(mylog[j].pitmaster);
          output.print(",");
          output.print(mylog[j].soll);
      
          for (int i=0; i < 3; i++)  {
            output.print(",");
            if (mylog[j].tem[i]/10 == INACTIVEVALUE)
              output.print(0);
            else
              output.print(mylog[j].tem[i]/10.0);   
          } 
          output.print("],\n");
        
          dothis--;
          //if (dothis < logstart) dothis = 0;
        }
        Serial.print("next: ");
        Serial.println(output.length());
      
        output.getBytes(buffer, maxLen); 
        return output.length();

      } else if (dothis == 0) {

        Serial.print("last: ");
      
        output.print(F("]);\nvar options = {\nhAxis: {gridlines: {color: 'white', count: 5}},\nwidth: 700,\nheight: 400,\n"));  //curveType: 'function',\n
        output.print(F("vAxes:{\n0:{title:'Pitmaster in %',ticks:[0,20,40,60,80,100],viewWindow:{min: 0},gridlines:{color:'transparent'},titleTextStyle:{italic:0}},\n1: {title: 'Temperatur in C', minValue: 0, gridlines: {color: 'transparent'}, titleTextStyle: {italic:0}},},\n"));
        output.print(F("series:{\n0: {targetAxisIndex: 0, color: 'black'},\n1: {targetAxisIndex: 1, color: 'red', lineDashStyle: [4,4]},\n2: {targetAxisIndex: 1, color: '#0C4C88'},\n3: {targetAxisIndex: 1, color: '#22B14C'},\n4: {targetAxisIndex: 1, color: '#5587A2'},\n},\n};\n"));
        output.print(F("var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));\nchart.draw(data, options);}\n</script>\n</head>\n"));
        output.print(F("<body>\n<font color=\"#000000\">\n<body bgcolor=\"#d0d0f0\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">"));
        output.print(F("\n<div id=\"curve_chart\"></div>\n</body>\n</html>"));

        Serial.println(output.length());
        dothis--;
        if (output.length() < maxLen) {
          output.getBytes(buffer, maxLen); 
          return output.length();
        } else  return 0;
         
      } else {
        dothis = 2;
        
        DPRINTF("[INFO]\tLogtime: %ums\r\n", millis()-vorher); 
        return 0;
      }
    });
    
    request->send(response);
}

*/


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// 
void server_setup() {

  MDNS.begin(sys.host.c_str());  // siehe Beispiel: WiFi.hostname(host); WiFi.softAP(host);
  DPRINTP("[INFO]\tOpen http://");
  DPRINT(sys.host);
    
  server.addHandler(&nanoWebHandler);
  server.addHandler(&bodyWebHandler);
  server.addHandler(&logHandler);
    
  server.on("/help",HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("https://github.com/WLANThermo-nano/WLANThermo_nano_Software/blob/master/README.md");
  }).setFilter(ON_STA_FILTER);
    

/*
    server.on("/plot", HTTP_GET, [](AsyncWebServerRequest *request) { 
      handlePlot(request, true);
    });
  */

      
  server.on("/info",[](AsyncWebServerRequest *request){
    FSInfo fs_info;
    SPIFFS.info(fs_info);
    request->send(200,"","totalBytes:" +String(fs_info.totalBytes) +
      " usedBytes:" + String(fs_info.usedBytes)+" blockSize:" + String(fs_info.blockSize)
      +" pageSize:" + String(fs_info.pageSize)
      +" heap:"+String(ESP.getFreeHeap())
      +" serial:"+String(ESP.getChipId(), HEX));
  });

  server.on("/god",[](AsyncWebServerRequest *request){
    sys.god =!sys.god;
    setconfig(eSYSTEM,{});
    if (sys.god) request->send(200, "text/plain", "GodMode aktiviert.");
    else request->send(200, "text/plain", "GodMode deaktiviert.");
  });

  server.on("/clearplot",[](AsyncWebServerRequest *request){
    log_count = 0; //TEST
    request->send(200, "text/plain", "true");
  });

  

  server.on("/setDC",[](AsyncWebServerRequest *request) { 
      if(request->hasParam("aktor")&&request->hasParam("dc")&&request->hasParam("val")){
        ESP.wdtDisable(); 
        bool dc = request->getParam("dc")->value().toInt();
        byte aktor = request->getParam("aktor")->value().toInt();
        int val = request->getParam("val")->value().toInt();
        DC_control(dc, aktor, val);
        DPRINTP("[INFO]\tDC-Test: ");
        DPRINTLN(val);
        ESP.wdtEnable(10);
        request->send(200, "text/plain", "true");
      } else request->send(200, "text/plain", "false");
  });

  server.on("/getRequest",[](AsyncWebServerRequest *request) { 
      if(request->hasParam("url")&&request->hasParam("method")&&request->hasParam("host")){
        //ESP.wdtDisable(); 
        //Serial.println(ESP.getFreeHeap());
        Serial.println("[REQUEST]\t/getRequest");
        myrequest.url = request->getParam("url")->value();
        myrequest.host = request->getParam("host")->value();
        myrequest.method = request->getParam("method")->value();
        getRequest(); 
        //ESP.wdtEnable(10);
        myrequest.request = request;
      } else request->send(200, "text/plain", "false");
  });

  server.on("/autotune",[](AsyncWebServerRequest *request) { 
      if(request->hasParam("cycle")&&request->hasParam("over")&&request->hasParam("timelimit")){
        ESP.wdtDisable(); 
        Serial.println(ESP.getFreeHeap());
        long limit = request->getParam("timelimit")->value().toInt();
        int over = request->getParam("over")->value().toInt();
        int cycle = request->getParam("cycle")->value().toInt();
        startautotunePID(cycle, true, over, limit);
        ESP.wdtEnable(10);
        request->send(200, "text/plain", "true");
      } else request->send(200, "text/plain", "false");
  });

  // to avoid multiple requests to ESP
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html"); // gibt alles im Ordner frei
    
  // 404 NOT found: called when the url is not defined here
  server.onNotFound([](AsyncWebServerRequest *request){
    request->send(404);
  });
      
  server.begin();
  DPRINTPLN("[INFO]\tHTTP server started");
  MDNS.addService("http", "tcp", 80);
}

