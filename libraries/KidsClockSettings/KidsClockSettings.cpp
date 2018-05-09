#include <math.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <KidsClockSettings.h>

static struct KidsClockSettings
{
  char ssid[SSID_SIZE];
  char pass[PASS_SIZE];
  char mainICALurl[MAIN_ICAL_URL_SIZE];
  char holidaysICALurl[HOLIDAYS_ICAL_URL_SIZE]; 
  char tz[TZ_SIZE];
  char indexfile[INDEX_FILE_SIZE];
} settings;

#define TOTAL_SIZE (sizeof(KidsClockSettings))

const char* getSSID()
{
  return settings.ssid;
}

const char* getWIFIKey()
{
  return settings.pass;
}

const char* getMainICALurl()
{
  return settings.mainICALurl;
}

const char* getHolidaysICALurl()
{
  return settings.holidaysICALurl;
}

const char* getTZ()
{
  return settings.tz;
}

const char* getIndexFile()
{
  return settings.indexfile;
}

void setKidsClockSettings(const char* newSSID,const char* newPass,const char* newMainICALurl,const char* newHolidaysICALurl,const char* newTZ, const char* newIndexFile)
{
  if (newSSID)
  {
    memcpy(settings.ssid,newSSID,SSID_SIZE-1);
    settings.ssid[SSID_SIZE-1]=0;
  }
    
  if (newPass)
  {
    memcpy(settings.pass,newPass,PASS_SIZE-1);
    settings.pass[PASS_SIZE-1]=0;
  }

  if (newMainICALurl)
  {
    memcpy(settings.mainICALurl,newMainICALurl,MAIN_ICAL_URL_SIZE-1);
    settings.mainICALurl[MAIN_ICAL_URL_SIZE-1]=0;
  }

  if (newHolidaysICALurl)
  {
    memcpy(settings.holidaysICALurl,newHolidaysICALurl,HOLIDAYS_ICAL_URL_SIZE-1);
    settings.holidaysICALurl[HOLIDAYS_ICAL_URL_SIZE-1]=0;
  }

  if (newTZ)
  {
    memcpy(settings.tz,newTZ,TZ_SIZE-1);
    settings.tz[TZ_SIZE-1]=0;
  }
  
  if (newIndexFile)
  {
    memcpy(settings.indexfile,newIndexFile,INDEX_FILE_SIZE-1);
    settings.indexfile[INDEX_FILE_SIZE-1]=0;
  }
}

bool loadKidsClockSettings()
{
  unsigned char* p=(unsigned char*)&settings;
  
  if (!EEPROM.begin(TOTAL_SIZE))
    return false;

  for (int i=0;i<TOTAL_SIZE;i++,p++)
    *p=EEPROM.read(i);
  return true;
}

bool saveKidsClockSettings()
{
  unsigned char* p=(unsigned char*)&settings;
  
  if (!EEPROM.begin(TOTAL_SIZE))
    return false;

  for (int i=0;i<TOTAL_SIZE;i++,p++)
    EEPROM.write(i,*p);
  return EEPROM.commit();
}
                                  