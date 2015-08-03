/*******************************************************************************
*  skRTClib.h - RTC-8564NB(リアルタイムクロック)関数のインクルードファイル     *
*                                                                              *
* ============================================================================ *
*  VERSION DATE        BY                    CHANGE/COMMENT                    *
* ---------------------------------------------------------------------------- *
*  1.00    2012-01-15  きむ茶工房(きむしげ)  Create                            *
*  1.10    2012-09-13  きむ茶工房            ArduinoIDE1.0.1に対応して変更     *
*******************************************************************************/
#ifndef skRTClib_h
#define skRTClib_h

#include "arduino.h"


#define RTC_ADRS 0B1010001              // ＲＴＣのスレーブアドレス


/*******************************************************************************
*	クラスの定義                                                              *
*******************************************************************************/
class skRTClib
{
  public:
    static byte InterFlag ;
         skRTClib() ;
    unsigned int bcd2bin(byte dt) ;
    unsigned int bin2bcd(unsigned int num) ;
    int begin(byte Inter,byte Year,byte Mon,byte mDay,byte wDay,byte Hour,byte Min,byte Sec) ;
    int sTime(byte Year,byte Mon,byte mDay,byte wDay,byte Hour,byte Min,byte Sec) ;
    int rTime(byte *tm) ;
    void cTime(byte *tm, byte *c) ;
    int SetTimer(byte sclk,byte count) ;
    int StopTimer() ;
    int SetAlarm(byte Hour,byte Min,byte mDay,byte wDay) ;
    int StopAlarm() ;
    int offAlarm() ;
  private:
    static byte Ctrl2 ;
    void set_ctime(byte tm,byte s,byte *a) ;
} ;

extern skRTClib RTC ;

#endif
