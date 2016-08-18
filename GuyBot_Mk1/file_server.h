#include "Arduino.h"

#include <vector>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>


class BotWebServer
{
public:
  typedef std::function<void(String&)> JSONProvider_t;

  void start();
  bool update();
  
  String formatBytes(size_t bytes);

  void AddJSONProvider( JSONProvider_t provider );

private:

  String getContentType(String filename);
  
  bool handleFileRead(String path);
  void handleFileUpload();
  void handleFileDelete();
  void handleFileCreate();
  void handleFileList();
  void handlePinMode();
  void handlePinWrite();

  std::vector<JSONProvider_t> m_JSONProviders;
  
  //holds the current upload
  File m_fsUploadFile;
  ESP8266WebServer m_server;
};


