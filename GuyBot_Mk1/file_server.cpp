#include "file_server.h"

#define DBG_OUTPUT_PORT Serial

void BotWebServer::start()
{
  //SERVER INIT
  //list directory
  m_server.on( "/list", HTTP_GET, std::bind( &BotWebServer::handleFileList, this) );
  
  //load editor
  m_server.on( "/edit", HTTP_GET, [this]()
  {
    if( !handleFileRead("/edit.htm") ) this->m_server.send( 404, "text/plain", "FileNotFound" );
  });
  
  //create file
  m_server.on( "/edit", HTTP_PUT, std::bind( &BotWebServer::handleFileCreate, this) );
  
  //delete file
  m_server.on( "/edit", HTTP_DELETE, std::bind( &BotWebServer::handleFileDelete, this) );
  
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  m_server.on("/edit", HTTP_POST, [this]()
  {
    this->m_server.send(200, "text/plain", ""); 
  }, std::bind( &BotWebServer::handleFileUpload, this) );

  m_server.on( "/pinmode", HTTP_POST, std::bind( &BotWebServer::handlePinMode, this ) );

  m_server.on( "/pinwrite", HTTP_POST, std::bind( &BotWebServer::handlePinWrite, this ) );

  //called when the url is not defined here
  //use it to load content from SPIFFS
  m_server.onNotFound([this]()
  {
    if(!handleFileRead(this->m_server.uri()))
    {
      this->m_server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  m_server.on("/all", HTTP_GET, [this]()
  {
    String json = "{";
    json += "\"heap\":"+String(ESP.getFreeHeap());
    json += ",\n \"analog\":"+String(analogRead(A0));
    json += ",\n \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    //json += ", \"gpio-in\":"+String((uint32_t)(((GPI) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    //json += ", \"gpio-out\":"+String((uint32_t)(((GPO) & 0xFFFF) | ((GP16O & 0x01) << 16)));

    for( JSONProvider* provider : m_JSONProviders ) 
    {
        provider->AppendMotorJSON( json );
    }

    /*for( int i = 0; i < 32; ++i )
    {
      json += ", \"gp-digitalRead" + String(i) + "\":"+String(digitalRead(i));
    }*/
    
    json += "}";
    this->m_server.send(200, "text/json", json);
    json = String();
  });
  
  m_server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
}


void BotWebServer::update()
{
  m_server.handleClient();
}

//format bytes
String BotWebServer::formatBytes(size_t bytes)
{
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}


void BotWebServer::AddJSONProvider( JSONProvider* provider )
{
  m_JSONProviders.push_back( provider );
}


String BotWebServer::getContentType(String filename)
{
  if(m_server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}


bool BotWebServer::handleFileRead(String path)
{
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.htm";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = m_server.streamFile(file, contentType);
    file.close();
    return sent > 0;
  }
  return false;
}


void BotWebServer::handleFileUpload()
{
  if(m_server.uri() != "/edit")
  {
    return;
  }
  
  HTTPUpload& upload = m_server.upload();
  if(upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if(!filename.startsWith("/"))
    {
      filename = "/"+filename;
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    m_fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  }
  else if(upload.status == UPLOAD_FILE_WRITE)
  {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(m_fsUploadFile)
    {
      m_fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if(upload.status == UPLOAD_FILE_END)
  {
    if(m_fsUploadFile)
    {
      m_fsUploadFile.close();
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}


void BotWebServer::handleFileDelete()
{
  if(m_server.args() == 0) return m_server.send(500, "text/plain", "BAD ARGS");
  String path = m_server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if(path == "/")
    return m_server.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return m_server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  m_server.send(200, "text/plain", "");
  path = String();
}


void BotWebServer::handleFileCreate()
{
  if(m_server.args() == 0)
    return m_server.send(500, "text/plain", "BAD ARGS");
  String path = m_server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if(path == "/")
    return m_server.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return m_server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return m_server.send(500, "text/plain", "CREATE FAILED");
  m_server.send(200, "text/plain", "");
  path = String();
}


void BotWebServer::handleFileList()
{
  if(!m_server.hasArg("dir")) {m_server.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = m_server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  m_server.send(200, "text/json", output);
}


void BotWebServer::handlePinMode()
{
  DBG_OUTPUT_PORT.println("handlePinMode");
  if(!m_server.hasArg("pin")) {m_server.send(500, "text/plain", "BAD ARGS"); return;}
  if(!m_server.hasArg("mode")) {m_server.send(500, "text/plain", "BAD ARGS"); return;}

  String pin = m_server.arg("pin");
  String mode = m_server.arg("mode");
  DBG_OUTPUT_PORT.println("handlePinMode pin:" + pin + " mode: " + mode);

  int iPin = 0;

  if( pin.equalsIgnoreCase( "builtinled" ) )
  {
    DBG_OUTPUT_PORT.printf("LED_BUILTIN is %d\n", LED_BUILTIN);
    iPin = LED_BUILTIN;
  }
  else
  {
    iPin = pin.toInt();
  }

  int iMode = OUTPUT;
  if( mode.equalsIgnoreCase( "input" ) )
  {
    iMode=INPUT;
  }
  else if( mode.equalsIgnoreCase( "output" ) )
  {
    iMode=OUTPUT;
  }
  else
  {
    m_server.send(500, "text/plain", "BAD ARGS mode must be input or output"); return;
  }

  DBG_OUTPUT_PORT.printf("Setting pin %d to %d\n", iPin, iMode );
  pinMode( iPin, iMode );
  m_server.send(200, "text/plain", "handlePinMode OK");
}


void BotWebServer::handlePinWrite()
{
  DBG_OUTPUT_PORT.println("handlePinWrite");
  if(!m_server.hasArg("pin")) {m_server.send(500, "text/plain", "BAD ARGS"); return;}
  if(!m_server.hasArg("value")) {m_server.send(500, "text/plain", "BAD ARGS"); return;}

  String pin = m_server.arg("pin");
  String value = m_server.arg("value");
  DBG_OUTPUT_PORT.println("handlePinWrite pin:" + pin + " value: " + value);

  int iPin = 0;
  if( pin.equalsIgnoreCase( "builtinled" ) )
  {
    DBG_OUTPUT_PORT.printf("LED_BUILTIN is %d\n", LED_BUILTIN);
    iPin = LED_BUILTIN;
  }
  else
  {
    iPin = pin.toInt();
  }

  int iValue = HIGH;
  if( value.equalsIgnoreCase( "high" ) )
  {
    iValue=HIGH;
  }
  else if( value.equalsIgnoreCase( "low" ) )
  {
    iValue=LOW;
  }
  else
  {
    m_server.send(500, "text/plain", "BAD ARGS value must be high or low"); return;
  }

  DBG_OUTPUT_PORT.println("digitalWrite " + String(iPin) + " " + String(iValue));
  digitalWrite( iPin, iValue );
  m_server.send(200, "text/plain", "handlePinWrite OK");
}



