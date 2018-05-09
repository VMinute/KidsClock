#include <Adafruit_SSD1306.h>

#include <Adafruit_GFX.h>
#include <gfxfont.h>

#include <HTTPClient.h>
#include <KidsClockSettings.h>
#include <RootCertificates.h>
#include <WiFi.h>

#include "time.h"

#define FONT_WIDTH  6
#define FONT_HEIGHT 8
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

#define PIR_GPIO      12

// macros
#define HHMMSS_IN_SECS(h,m,s) (h*3600+m*60+s)

// time that screen is turned on after movement has been detected
#define SCREEN_TIMEOUT HHMMSS_IN_SECS(0,1,0)

#define CLOCK_POLL_TIME     HHMMSS_IN_SECS(2,00,0)
#define HOLIDAYS_POLL_TIME  HHMMSS_IN_SECS(12,0,0)
#define MAIN_POLL_TIME      HHMMSS_IN_SECS(0,10,0)
#define INDEXFILE_POLL_TIME HHMMSS_IN_SECS(1,0,0)

#define WIFI_TIMEOUT        HHMMSS_IN_SECS(0,0,30)

#define DAY_IN_SECS         (24*3600)

// bitmaps
const unsigned char holidayImage [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 0x00, 
  0x00, 0x00, 0x0c, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x60, 0x00, 0x00, 0x00, 0x00, 0x18, 0x70, 
  0x03, 0xe0, 0x00, 0x00, 0x30, 0x30, 0x0f, 0xf0, 0x00, 0x00, 0x7c, 0xf8, 0x0c, 0x30, 0x00, 0x00, 
  0x3c, 0xf0, 0x18, 0x18, 0x00, 0x00, 0x18, 0x60, 0x18, 0x18, 0x00, 0x00, 0x38, 0x30, 0x18, 0x18, 
  0x00, 0x00, 0x70, 0x38, 0x1c, 0x18, 0x00, 0x00, 0x60, 0x18, 0x0e, 0x70, 0x00, 0x00, 0xf0, 0x3c, 
  0x07, 0xe0, 0x00, 0x00, 0x70, 0x3c, 0x03, 0xc0, 0x00, 0x00, 0x70, 0x38, 0x00, 0x00, 0x00, 0x00, 
  0x60, 0x18, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0c, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x0c, 0x00, 0x00, 
  0x00, 0x01, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x03, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x01, 
  0xff, 0xf0, 0x00, 0x00, 0x00, 0x1f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 
  0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xfe, 0x00, 0x1e, 0x00, 0x00, 0x00, 
  0xff, 0xf0, 0x78, 0x00, 0x00, 0x00, 0x01, 0xfe, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 
  0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char normalImage [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 
  0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 
  0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x01, 0xf8, 0x1f, 0x80, 0x00, 
  0x00, 0x00, 0x31, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x33, 0xcc, 
  0x00, 0x00, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x37, 0xec, 0x00, 0x00, 0x00, 0x00, 
  0x30, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x31, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 
  0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x07, 0xff, 0xff, 
  0xe0, 0x00, 0x00, 0x0f, 0xfe, 0x7f, 0xf0, 0x00, 0x00, 0x3f, 0xf8, 0x1f, 0xfc, 0x00, 0x00, 0x7f, 
  0xf8, 0x1f, 0xfe, 0x00, 0x00, 0xff, 0xfb, 0x1f, 0xff, 0x00, 0x01, 0xff, 0xf8, 0x1f, 0xff, 0x80, 
  0x07, 0xff, 0xf8, 0x1f, 0xff, 0xe0, 0x0f, 0xff, 0xfe, 0x7f, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xff, 
  0xff, 0xf8, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xff, 
  0xfc, 0x3f, 0xff, 0x00, 0x00, 0xff, 0xf8, 0x1f, 0xff, 0x00, 0x00, 0xff, 0xf0, 0x0f, 0xff, 0x00, 
  0x00, 0xff, 0xf0, 0x0f, 0xff, 0x00, 0x00, 0xe0, 0x70, 0x0e, 0x07, 0x00, 0x00, 0xe0, 0x70, 0x0e, 
  0x07, 0x00, 0x00, 0xe0, 0x70, 0x0e, 0x07, 0x00, 0x00, 0xe0, 0x70, 0x0e, 0x07, 0x00, 0x00, 0xff, 
  0xf0, 0x0f, 0xff, 0x00, 0x00, 0xe0, 0x70, 0x0e, 0x07, 0x00, 0x00, 0xe0, 0x70, 0x0e, 0x07, 0x00, 
  0x00, 0xff, 0xf0, 0x0f, 0xff, 0x00, 0x00, 0xff, 0xf0, 0x0f, 0xff, 0x00, 0x00, 0xff, 0xf0, 0x0f, 
  0xff, 0x00, 0x00, 0xff, 0xf0, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 
  0xff, 0xff, 0xff, 0xc0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char nightImage [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 
  0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x05, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x0e, 0x00, 0x00, 
  0x00, 0xf8, 0x00, 0x0f, 0x00, 0x00, 0x01, 0xf8, 0x00, 0x04, 0x00, 0x00, 0x01, 0xf8, 0x08, 0x00, 
  0x00, 0x00, 0x01, 0xfc, 0x3e, 0x00, 0x00, 0x00, 0x01, 0xfc, 0x1c, 0x00, 0x00, 0x00, 0x01, 0xfc, 
  0x1e, 0x00, 0x00, 0x00, 0x03, 0xfc, 0x2c, 0x00, 0x00, 0x00, 0x03, 0xfe, 0x00, 0x01, 0x40, 0x00, 
  0x01, 0xfe, 0x00, 0x03, 0xc0, 0x00, 0x01, 0xfe, 0x00, 0x07, 0xe0, 0x00, 0x01, 0xff, 0x00, 0x01, 
  0xc0, 0x00, 0x01, 0xff, 0x80, 0x03, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x40, 0x00, 0x00, 0xff, 
  0xc0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 
  0x00, 0x7f, 0xfc, 0x00, 0x00, 0x80, 0x00, 0x3f, 0xff, 0x00, 0x03, 0x00, 0x00, 0x3f, 0xff, 0xe0, 
  0x1f, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x07, 
  0xff, 0xff, 0xf8, 0x00, 0x00, 0x01, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 
  0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char wifiOKImage [] PROGMEM = {
  // 'wifiok'
  0x03, 0xc0, 0x0f, 0xf0, 0x1c, 0x38, 0x30, 0x0c, 0x63, 0xc6, 0x67, 0xf2, 0x0c, 0x30, 0x18, 0x18, 
  0x13, 0xc8, 0x07, 0xe0, 0x04, 0x20, 0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 
};

const unsigned char wifiKOImage [] PROGMEM = {
  // 'wifiko'
  0x83, 0xc1, 0x4f, 0xf2, 0x24, 0x24, 0x10, 0x08, 0x49, 0x92, 0x64, 0x22, 0x02, 0x40, 0x19, 0x18, 
  0x10, 0x88, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10, 0x11, 0x88, 0x21, 0x84, 0x40, 0x02, 0x80, 0x01, 
};

// custom image for special events etc.
unsigned char customImage[(48*48)/8];

// types

// function to parse additional info inside calendar & events
typedef void (* Parse_Func)(String line,bool inCalendar,bool inEvent);

typedef struct URL_Map
{
  String    key;
  String    url;
  URL_Map*  next;
} URL_Map;

// const values

// server used to sync time
const char* ntpServer = "pool.ntp.org";

// global vars

// display
Adafruit_SSD1306 display;

// info about next holiday
time_t hdaystart,hdayend;
String hdaysummary;
String hdaydescription;

// info about next event
time_t nextstart,nextend;
String nextsummary;
String nextdescription;

// wake up times
time_t normalwt=HHMMSS_IN_SECS(8,0,0);    // 8:00 in seconds
time_t holidayswt=HHMMSS_IN_SECS(8,30,0); // 8:30 in seconds

// bed time
time_t bedTime=HHMMSS_IN_SECS(21,30,0);   // 21:30 in seconds

// time when clock has ben synced
time_t lastClockSync=0;

// time when calendars have been checked
time_t lastMainParse=0;
time_t lastHolidaysParse=0;
time_t lastIndexFileParse=0;

// time for wake-up
time_t  todaysWakeupTime=0;
time_t  todaysBedTime=0;

// time used to check movement from PIR sensor
time_t  lastMovement=0;

// current time
time_t now;

// flag to enable screen output
bool screenEnabled=true;

// flag to show network state
bool lastConnectionOK=false;

// flag to enable custom image
bool customImageIsValid=false;

// map to find custom images urls
URL_Map* imageIndex=NULL;

// handles code 302 to download files via redirect
WiFiClient* sendRequestWithRedirect(HTTPClient* http,const char* url)
{
  const char* headers[]={"Location"};
  int         code;
  WiFiClient* returned=NULL;
  
  http->collectHeaders(headers,1);
  http->begin(url,getCertificate(url));

  while ((code=http->GET())!=HTTP_CODE_OK)
  {
    if (code==HTTP_CODE_FOUND)
    {
      String newurl=http->header("Location");
      http->end();     

      if (newurl.length()!=0)
      {
        Serial.println("Redirected to:");
        Serial.println(newurl);

        http->collectHeaders(headers,1);
        http->begin(newurl,getCertificate(newurl.c_str()));
      }
      else
      {
        Serial.println("Invalid redirect");
        return NULL;        
      }
    }
    else
    {
      Serial.print("HTTP error ");
      Serial.println(code,DEC);
      return NULL;
    }
  }
  Serial.println("OK");
  returned=http->getStreamPtr();
  returned->setTimeout(60000);

  return returned;
}

// parses a full date string into a time_t
time_t parseDateString(String datestring)
{
  struct tm date;
  bool isUTC=false;

  memset(&date,0,sizeof(struct tm));

  //Serial.println(datestring);

  date.tm_year=datestring.substring(0,4).toInt()-1900;
  date.tm_mon=datestring.substring(4,6).toInt()-1;
  date.tm_mday=datestring.substring(6,8).toInt();

  if (datestring.length()>8)
  {
    if (datestring.charAt(8)!='T')
    {
      Serial.println("T separator expected between date and time");
      return -1;
    }
      
    date.tm_hour=datestring.substring(9,11).toInt();
    date.tm_min=datestring.substring(11,13).toInt();
    date.tm_sec=datestring.substring(13,15).toInt();
  }

  // check for "Z" at the end of time entry, marking time as UTC
  if (datestring.endsWith("Z"))
    isUTC=true;

  /*
  Serial.print(date.tm_year,DEC);
  Serial.print("/");
  Serial.print(date.tm_mon,DEC);
  Serial.print("/");
  Serial.print(date.tm_mday,DEC);
  Serial.print("-");
  Serial.print(date.tm_hour,DEC);
  Serial.print(":");
  Serial.print(date.tm_min,DEC);
  Serial.print(":");
  Serial.println(date.tm_sec,DEC);
  */

  // if time is already UTC we should add back _timezone offset
  if (isUTC)
    return mktime(&date)-_timezone;
  return mktime(&date);
}

bool getNextEvent(WiFiClient* stream,time_t* startdate,time_t* enddate,String* summary,String* description, Parse_Func func)
{
  bool inCalendar=false;
  bool inEvent=false;
  bool eventFound=false;
  time_t evtstart;
  time_t evtend;
  String evtsummary;
  String evtdescription;
  time_t mindiff=LONG_MAX;
  time_t diff;
  time_t newdata=time(NULL);

  String data="";
  
  while (stream->connected())
  {    
    int lfindex;
    int datalen;

    datalen=data.length();
    data+=stream->readString();

    // no new data, check timeout
    if (datalen==data.length())
    {
      if (time(NULL)-newdata>30000)
        return false;
      
      continue;
    }

    newdata=time(NULL);
    
    Serial.print("Data: ");
    Serial.println(data);
    
    // parses all lines in current data
    while ((lfindex=data.indexOf('\n'))!=-1)
    {    
      String line=data.substring(0,lfindex);
  
      data=data.substring(lfindex+1);
  
      if (line.endsWith("\r"))
        line.remove(line.length()-1);
        
      line.trim();

      Serial.print("Parsing: ");
      Serial.println(line);
  
      if (func)
        (*func)(line,inCalendar,inEvent);
  
      if (line.compareTo("BEGIN:VCALENDAR")==0)
      {
        inCalendar=true;
        Serial.println("Found valid calendar.");
        continue;
      }
      if (line.compareTo("END:VCALENDAR")==0)
      {
        inCalendar=false;
        Serial.println("Parsing completed.");
        return eventFound;
      }
  
      if (!inCalendar)
        continue;
  
      if (line.compareTo("BEGIN:VEVENT")==0)
      {
        // clears state vars
        inEvent=true;
        evtsummary="";
        evtdescription="";
        evtstart=0;
        evtend=0;
        continue;
      }
      if (line.compareTo("END:VEVENT")==0)
      {
        inEvent=false;
  
        Serial.println("Event:");
        Serial.println(evtsummary);
        Serial.println(evtdescription);
        Serial.print("from ");
        Serial.print(evtstart);
        Serial.print(" to ");
        Serial.println(evtend);
        
        if (evtstart>now)
        {
          diff=evtstart-now;
        }
        else
        {
          if (evtend<now)
            continue;
  
          diff=0;
        }
  
        if (diff<mindiff)
        {
          mindiff=diff;
          eventFound=true;
          *startdate=evtstart;
          *enddate=evtend;
          *summary=evtsummary;
          *description=evtdescription;
        }
        continue;
      }
  
      if (!inEvent)
        continue;
  
      // two different formats (or at least two)
      if (line.startsWith("DTSTART:"))
      {
        evtstart=parseDateString(line.substring(8,line.length()));      
      }    
      if (line.startsWith("DTSTART;VALUE=DATE:"))
      {
        evtstart=parseDateString(line.substring(19,line.length()));      
      }
      if (line.startsWith("DTEND:"))
      {
        evtend=parseDateString(line.substring(6,line.length()));            
      }
      if (line.startsWith("DTEND;VALUE=DATE:"))
      {
        evtend=parseDateString(line.substring(17,line.length()));            
      }
      if (line.startsWith("DESCRIPTION:"))
      {
        evtdescription=line.substring(12,line.length());            
      }
      if (line.startsWith("SUMMARY:"))
      {
        evtsummary=line.substring(8,line.length());            
      }
    }
  }
  return false;
}

bool parseHolidaysCalendar()
{
  HTTPClient  http;
  bool        retcode=false;

  Serial.println("Downloading holiday calendar");
  Serial.println(getHolidaysICALurl());
 
  WiFiClient* stream=sendRequestWithRedirect(&http,getHolidaysICALurl());

  if (!stream)
  {
    http.end();
    return false;
  }
  
  if (getNextEvent(stream,&hdaystart,&hdayend,&hdaysummary,&hdaydescription,NULL))
  {
    Serial.print("Found next holiday event:");
    Serial.println(hdaysummary);
    Serial.print("From ");
    Serial.print(hdaystart);
    Serial.print(" to ");
    Serial.println(hdayend);
    retcode=true;
  }
  else
  {
    Serial.println("No events found in calendar.");
  }
  http.end();   
  return retcode;
}

int parseTime(String hhmm)
{
    hhmm.trim();

    int pos=hhmm.indexOf(":");

    if (pos==-1)
      return -1;

    int hh=hhmm.substring(0,pos).toInt();
    int mm=hhmm.substring(pos+1,hhmm.length()).toInt();

    if ((hh<0)||(hh>=24))
      return -1;

    if ((mm<0)||(mm>=60))
      return -1;
      
    return hh*3600+mm*60;  
}

void parseSetting(String setting)
{
  int dummy;

  Serial.println(setting);
  
  // regular wake up time
  if (setting.startsWith("N:"))
  {    
    dummy=parseTime(setting.substring(2,setting.length()));

    if (dummy==-1)
    {
      Serial.println("Invalid value for N setting");
    }
    normalwt=dummy;  
    Serial.print("Normal time: ");
    Serial.println(normalwt); 
  }
  else if (setting.startsWith("H:"))
  {    
    dummy=parseTime(setting.substring(2,setting.length()));

    if (dummy==-1)
    {
      Serial.println("Invalid value for H setting");
    }
    holidayswt=dummy;   
    Serial.print("Holidays time: ");
    Serial.println(holidayswt); 
  }
  else if (setting.startsWith("B:"))
  {    
    dummy=parseTime(setting.substring(2,setting.length()));

    if (dummy==-1)
    {
      Serial.println("Invalid value for B setting");
    }
    bedTime=dummy;   
    Serial.print("Bed time: ");
    Serial.println(bedTime); 
  }
}

void mainCalendarParse(String line,bool inCalendar,bool inEvent)
{
  // currently we check only calendar attributes, not events
  if (inEvent)
    return;

  if (line.startsWith("X-WR-CALDESC:"))
  {
    String  parsed=line.substring(13,line.length());
    String  setting;
    int     pos;

    while (parsed.length())
    {
      if ((pos=parsed.indexOf("\\n"))==-1)
      {
        setting=parsed;
        parsed="";
      }
      else
      {
        setting=parsed.substring(0,pos);
        parsed=parsed.substring(pos+2,parsed.length());
      }

      setting.trim();
      parseSetting(setting);
    }    
  }
}

bool loadCustomImage(String* key)
{
  URL_Map* p=imageIndex;
  HTTPClient    http;
  bool          retcode=false;
  unsigned char header[14];

  key->toLowerCase();
  
  while (p!=NULL)
  {    
    if (p->key.compareTo(*key)==0)
      break;
      
    p=p->next;
  }

  if (!p)
    return false;
  
  Serial.println("Downloading custom image");
  Serial.println(p->url);
  
  WiFiClient* stream=sendRequestWithRedirect(&http,p->url.c_str());

  if (!stream)
  {
    http.end();
    return false;
  }

  if (stream->readBytes(header,14)!=14)
  {  
    Serial.println("Cannot read data.");
    http.end();   
    return false;
  }

  // BMPs start with those two chars
  if ((header[0]!=0x42)||(header[1]!=0x4D))
  {  
    Serial.println("Invalid BMP header.");
    http.end();   
    return false;
  }

  // this is where bitmap data begins
  if (header[10]!=62)
  {  
    Serial.println("Invalid BMP format, save it with MS-PAINT as monochrome bitmap.");
    http.end();   
    return false;
  }

  // skip till offset 62 (we already loaded header)
  for (int i=0;i<(62-14);i++)
    stream->read();

  // data is 64bit aligned and BMPs are saved upside down
  for (int i=47;i>=0;i--)
  {
    stream->readBytes(customImage+i*6,6);
    stream->read();
    stream->read();
  }     
     
  retcode=true;  
  http.end();   
  return retcode;    
}

bool parseMainCalendar()
{
  HTTPClient  http;
  bool        retcode=false;

  Serial.println("Downloading main calendar");
  Serial.println(getMainICALurl());
  
  WiFiClient* stream=sendRequestWithRedirect(&http,getMainICALurl());

  if (!stream)
  {
    http.end();   
    return false;
  }

  if (getNextEvent(stream,&nextstart,&nextend,&nextsummary,&nextdescription,mainCalendarParse))
  {
    Serial.println("Calendar parsed.");
    Serial.print("Next event: ");
    Serial.println(nextsummary);
    customImageIsValid=loadCustomImage(&nextdescription);
    retcode=true;
  }
  else
  {
    Serial.println("No events found in calendar.");
    retcode=true;
  }
  http.end();   
  return retcode;  
}

void clearImageIndex()
{
  if (imageIndex)
  {
    URL_Map* q=imageIndex;
    URL_Map* p=imageIndex->next;
    
    for (;p!=NULL;q=p,p=p->next)
      delete q;

    delete imageIndex;
    imageIndex=NULL;
  }
}

bool parseIndexFile()
{
  HTTPClient  http;
  bool        retcode=false;

  Serial.println("Downloading index file");
  Serial.println(getIndexFile());

  WiFiClient* stream=sendRequestWithRedirect(&http,getIndexFile());
  String data="";
  time_t newdata;

  if (!stream)
  {
    http.end();   
    return false;
  }

  clearImageIndex();

  newdata=time(NULL);
  
  while (stream->connected())
  {
    int lfindex;
    int datalen;

    datalen=data.length();
    data+=stream->readString();

    // no new data, check timeout
    if (datalen==data.length())
    {
      if (time(NULL)-newdata>30000)
        return false;
      
      continue;
    }

    newdata=time(NULL);
    
    Serial.print("Data: ");
    Serial.println(data);
    
    // parses all lines in current data
    while ((lfindex=data.indexOf('\n'))!=-1)
    {    
      String    line=data.substring(0,lfindex);   
      int       index;
      URL_Map*  item;

      data=data.substring(lfindex+1);
  
      if (line.endsWith("\r"))
        line.remove(line.length()-1);
        
      line.trim();
      line.toLowerCase();

      if (line.equals("end"))
      {
        Serial.println("Index processed.");
        http.end();   
        return true;  
      }
  
      index=line.indexOf(":");
  
      if (index==-1)
      {
        Serial.println("Invalid entry:");
        Serial.println(line);
        continue;
      }
  
      item=new URL_Map();
  
      item->key=line.substring(0,index);
      item->key.trim();
      item->url=line.substring(index+1,line.length());
      item->url.trim();
      item->next=imageIndex;
  
      Serial.print("Found image: ");
      Serial.print(item->key);
      Serial.print(" - ");
      Serial.println(item->url);
  
      imageIndex=item;
    }
  }
  http.end();   
  return false;  
}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


bool updateFromNetwork(bool syncTime,bool pollMain,bool pollHolidays,bool pollIndexFile)
{
  bool retval=true;
  
  if ((!syncTime)&&(!pollMain)&&(!pollHolidays)&&(!pollIndexFile))
    return true;
    
  //try to connect WiFi   
  Serial.printf("Connecting to %s ", getSSID());

  WiFi.mode(WIFI_STA);
  WiFi.begin(getSSID(), getWIFIKey());
   
  while ((WiFi.status() != WL_CONNECTED)&&((now==0)||((time(NULL)-now)<WIFI_TIMEOUT)))
  {
      Serial.print(".");
      delay(1000);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Connection timeout.");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);     
    return false;
  }
  
  Serial.println(" CONNECTED"); 

  if (syncTime)
  {
    struct tm timeinfo;
    //init and get the time
    configTzTime(getTZ(), ntpServer);

    if(getLocalTime(&timeinfo))
      lastClockSync=time(NULL);
    else
      retval=false;
  } 

  if (pollIndexFile)
  {
    if (parseIndexFile())
      lastIndexFileParse=time(NULL);
    else
      retval=false;
  }

  if (pollHolidays)
  {
    if (parseHolidaysCalendar())
      lastHolidaysParse=time(NULL);
    else
      retval=false;
  }

  if (pollMain)
  {
    if (parseMainCalendar())
      lastMainParse=time(NULL);
    else
      retval=false;
  }

 //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF); 
  Serial.println("WiFi disconnected.");
  return retval;
}    

bool checkHoliday()
{  
  // check if today is an holiday
  if ((hdaystart<now)&&(hdayend>now))
  {
    return true;
  }
  else
  {    
    return false;
  }  
}

void updateScreen()
{
  time_t      todaysBedTime,todaysWakeupTime;  
  time_t      midnight,nextMidnight;
  bool        nightMode=false;
  bool        useCustomImage=false;
  char        timestr[6];
  char        datestr[11];
  struct tm   tmtime;
  String      msg;
  bool        isSatSun=false;
  
  if (!screenEnabled)
  {
    display.clearDisplay();
    display.display();
    return;
  }    

  getLocalTime(&tmtime);
  
  sprintf(timestr,"%02d:%02d",tmtime.tm_hour,tmtime.tm_min);
  sprintf(datestr,"%02d/%02d/%04d",tmtime.tm_mday,tmtime.tm_mon+1,tmtime.tm_year+1900);

  msg=checkHoliday()?hdaysummary:datestr;

  if ((tmtime.tm_wday==0)||(tmtime.tm_wday==6))
    isSatSun=true;

  // calculate local midnight as time_t
  tmtime.tm_hour=0;
  tmtime.tm_min=0;
  tmtime.tm_sec=0;

  midnight=mktime(&tmtime);

  // calc bed and wakeup times
  todaysBedTime=midnight+bedTime;
  todaysWakeupTime=midnight+((isSatSun||checkHoliday())?holidayswt:normalwt);
  
  // check if we have a special event today
  tmtime.tm_hour=23;
  tmtime.tm_min=59;
  tmtime.tm_sec=59;

  nextMidnight=mktime(&tmtime);

  if ((nextstart>midnight)&&(nextstart<nextMidnight)) 
  {
    todaysWakeupTime=nextstart;
    useCustomImage=true;
    msg=nextsummary;
  }

  if ((now<todaysWakeupTime)||(now>todaysBedTime))
    nightMode=true;

  display.clearDisplay();
  display.setTextSize(2);
  
  if (nightMode)
  {
    display.fillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,BLACK);
    display.drawBitmap(8,0,nightImage,48,48,WHITE);    
    display.setTextColor(WHITE);
  }
  else
  {
    display.fillRect(0,0,SCREEN_WIDTH,64,WHITE);
    display.setTextColor(BLACK);
    if ((!useCustomImage)||(!customImageIsValid))
      display.drawBitmap(8,0,(isSatSun|checkHoliday())?holidayImage:normalImage,48,48,BLACK);    
    else
      display.drawBitmap(8,0,customImage,48,48,BLACK);    
  }

  // draws wi-fi status bitmap
  display.drawBitmap(112,0,lastConnectionOK?wifiOKImage:wifiKOImage,16,16,nightMode?WHITE:BLACK);  

  display.setCursor(66,24);
  display.print(timestr);
  display.setTextSize(1);

  if (msg.length()<(SCREEN_WIDTH/FONT_WIDTH))
  {
    // less than one line, display it centered
    display.setCursor((SCREEN_WIDTH-(msg.length()*FONT_WIDTH))>>1,SCREEN_HEIGHT-(FONT_HEIGHT*2)+FONT_HEIGHT);
    display.print(msg);
  }
  else
  {
    // split in two lines
    String msg1,msg2;
    int i;

    // use space to split words
    msg1=msg.substring(0,(SCREEN_WIDTH/FONT_WIDTH));

    for (i=(SCREEN_WIDTH/FONT_WIDTH)-1;i>0;i++)
    {
      if (msg1.charAt(i)==' ')
        break;
    }

    // no spaces 
    if (i==0)
    {
      msg1=msg.substring(0,(SCREEN_WIDTH/FONT_WIDTH));
      msg2=msg.substring((SCREEN_WIDTH/FONT_WIDTH),(SCREEN_WIDTH/FONT_WIDTH));
    }
    else
    {
      msg1=msg.substring(0,i);
      msg2=msg.substring(i,(SCREEN_WIDTH/FONT_WIDTH));
    }  
    
    int linelen1=msg1.length()*FONT_WIDTH;
    int linelen2=msg2.length()*FONT_WIDTH;
        
    display.setCursor((SCREEN_WIDTH-(linelen1*FONT_WIDTH))>>1,SCREEN_HEIGHT-FONT_HEIGHT*2);
    display.print(msg1);        

    display.setCursor((SCREEN_WIDTH-(linelen2*FONT_WIDTH))>>1,SCREEN_HEIGHT-FONT_HEIGHT);
    display.print(msg2);        
  }
    
  display.display();
  printLocalTime();
}

void setup()
{
  // used for debugging
  Serial.begin(115200);

  // configures pin for PIR sensor
  pinMode(PIR_GPIO,INPUT);

  // loads setting from EEPROM
  loadKidsClockSettings(); 
  
  // Initialize display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting to");
  display.println(getSSID());  
  display.display();
  
  display.clearDisplay();
  display.setCursor(0,0);

  now=0;

  while (!updateFromNetwork(true,false,false,false))
  {
    display.println("Connection failure");
    display.display();
  }

  display.println("Device connected");
  display.display();

  // forces screen to stay on for first update
  lastMovement=time(NULL);
  screenEnabled=true;
}

void loop()
{  
  bool syncTime=false;
  bool pollMain=false;
  bool pollHolidays=false;
  bool pollIndex=false;

  // no need to repeat the loop continously if PIR GPIO is active
  while (now==time(NULL))
    delay(500);
    
  now=time(NULL);

  Serial.print("Now: ");
  Serial.println(now);
  
  // we woke up for PIR (or it's first loop)
  if (digitalRead(PIR_GPIO))
  {
    lastMovement=now;

    if (!screenEnabled)
    {
      Serial.println("Movement detected, enabling screen");
      screenEnabled=true;
    }
  }
  else
  {
    if ((now-lastMovement)>=SCREEN_TIMEOUT)
    {
      if (screenEnabled)
      {
        Serial.println("Disabling screen");
        screenEnabled=false;      
      }
    }
  }

  if (now-lastClockSync>=CLOCK_POLL_TIME)
    syncTime=true;
    
  if (now-lastMainParse>=MAIN_POLL_TIME)
    pollMain=true;

  if (now-lastHolidaysParse>=HOLIDAYS_POLL_TIME)
    pollHolidays=true;

  if (now-lastIndexFileParse>=INDEXFILE_POLL_TIME)
    pollIndex=true;

  // show correct time
  updateScreen();

  if ((syncTime)||(pollMain)||(pollHolidays)||(pollIndex))
  {
    lastConnectionOK=updateFromNetwork(syncTime,pollMain,pollHolidays,pollIndex);
    // update screen anyway, in case something has changed
    updateScreen();
  }

  // if execution took less than one minute we poll the PIR gpio until next minute
  if (time(NULL)-now<HHMMSS_IN_SECS(0,1,0))
  {
    time_t nextminute=time(NULL);
    
    nextminute=((nextminute+HHMMSS_IN_SECS(0,1,0))/HHMMSS_IN_SECS(0,1,0))*HHMMSS_IN_SECS(0,1,0);  
    while ((time(NULL)<nextminute)&&(!digitalRead(PIR_GPIO)))
      delay(100);  
  }
}
