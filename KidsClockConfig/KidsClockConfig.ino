#include <KidsClockSettings.h>

char ssid[SSID_SIZE];
char pass[PASS_SIZE];
char mainICALurl[MAIN_ICAL_URL_SIZE]; 
char holidaysICALurl[HOLIDAYS_ICAL_URL_SIZE]; 
char tz[TZ_SIZE];
char indexfile[INDEX_FILE_SIZE];

static void readStringFromSerial(char* s,int len)
{
  int p=0;
  
  for (;;)
  {
    int c;
    // wait for a character
    while (Serial.available()==0)
      ;

    switch(c=Serial.read())
    {
      case '\r':
      case '\n':
        s[p]=0;
        return;
      default:
        s[p]=(char)c;
        Serial.write(c);
    }
    p++;
    if (p==len)
      return;
  }
}

static void readCertificateFromSerial(char* s,int len)
{
  int p=0;
  
  for (;;)
  {
    int c;
    // wait for a character
    while (Serial.available()==0)
      ;

    switch(c=Serial.read())
    {
      case '*':
        s[p]=0;
        return;
      default:
        s[p]=(char)c;
        Serial.write(c);
    }
    p++;
    if (p==len)
      return;
  }
}

void setup() { 
  // serial must be initialized
  Serial.begin(115200);  
  Serial.println("Insert configuration information, just send empty line to keep current value.");

  loadKidsClockSettings();

  // cleans up serial buffer
  while (Serial.available())
    Serial.read();

  // read SSID from console  
  Serial.print("SSID: ");
  Serial.println(getSSID());
  Serial.println("Insert SSID: ");
  readStringFromSerial(ssid,SSID_SIZE-1);

  while (Serial.available())
    Serial.read();
  
  // read password
  Serial.println("");
  Serial.print("WIFI key: ");
  Serial.println(getWIFIKey());
  Serial.println("Insert WIFI key: ");
  readStringFromSerial(pass,PASS_SIZE-1);

  // read ical url
  Serial.println("");
  Serial.print("ICAL main: ");
  Serial.println(getMainICALurl());
  Serial.println("Insert main ICAL url: ");
  readStringFromSerial(mainICALurl,MAIN_ICAL_URL_SIZE-1);

  // read holidays ical url
  Serial.println("");
  Serial.print("ICAL holidays: ");
  Serial.println(getHolidaysICALurl());
  Serial.println("Insert holidays ICAL url: ");
  readStringFromSerial(holidaysICALurl,HOLIDAYS_ICAL_URL_SIZE-1);

  // read TZ info
  Serial.println("");
  Serial.print("TZ: ");
  Serial.println(getTZ());
  Serial.println("Insert TZ: ");
  readStringFromSerial(tz,TZ_SIZE-1);

  Serial.println("");
  Serial.print("IndexFile: ");
  Serial.println(getIndexFile());
  Serial.println("Insert index file: ");
  readStringFromSerial(indexfile,INDEX_FILE_SIZE-1);

  setKidsClockSettings((*ssid)?ssid:NULL,(*pass)?pass:NULL,(*mainICALurl)?mainICALurl:NULL,(*holidaysICALurl)?holidaysICALurl:NULL,(*tz)?tz:NULL,(*indexfile)?indexfile:NULL);
  if (saveKidsClockSettings())
    Serial.println("Settings saved.");
}

void loop()
{
}


