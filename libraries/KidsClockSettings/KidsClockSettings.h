#ifndef KidsClockSETTINGS_H_
#define KidsClockSETTINGS_H_

#define SSID_SIZE               32
#define PASS_SIZE              128
#define MAIN_ICAL_URL_SIZE     256
#define HOLIDAYS_ICAL_URL_SIZE  256 
#define TZ_SIZE                 128
#define INDEX_FILE_SIZE          256

const char* getSSID();
const char* getWIFIKey();
const char* getMainICALurl();
const char* getHolidaysICALurl();
const char* getTZ();
const char* getIndexFile();

void setKidsClockSettings(const char* newSSID,const char* newPass,const char* newMainICALurl,const char* newHolidaysICALurl,const char* newTZ,const char* indexFile);
bool saveKidsClockSettings();
bool loadKidsClockSettings();

#endif //KidsClockSETTINGS_H_