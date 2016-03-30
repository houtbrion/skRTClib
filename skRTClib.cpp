/*******************************************************************************
*  skRTClib - RTC-8564NB(リアルタイムクロック)関数ライブラリ                   *
*                                                                              *
*    begin     - ＲＴＣの初期化を行う処理                                      *
*    sTime     - ＲＴＣに日付と時刻を書き込む処理                              *
*    rTime     - ＲＴＣから現在の日付と時刻を読み取る処理                      *
*    cTime     - ＲＴＣの日付と時刻を文字列に変換する処理                      *
*    SetTimer  - 定周期タイマーの設定をする処理（タイマーの開始）              *
*    StopTimer - 定周期タイマー機能を無効にする処理（タイマーの終了）          *
*    SetAlarm  - アラームの日時を設定する処理（アラームの開始）                *
*    StopAlarm - アラーム機能を無効にする処理（アラーム終了）                  *
*    offAlarm  - アラームの発生を解除する処理（アラームは引き続き有効）        *
*    bcd2bin   - ＢＣＤをバイナリ(10進数)に変換する処理                        *
*    bin2bcd   - バイナリ(10進数)をＢＣＤに変換する処理                        *
*                                                                              *
* ============================================================================ *
*   VERSION  DATE        BY             CHANGE/COMMENT                         *
* ---------------------------------------------------------------------------- *
*   1.00     2012-01-15  きむ茶工房     Create                                 *
*   1.01     2012-01-24  きむ茶工房     rTime()のバグ修正(ans=0の行追加)       *
*   1.10     2012-09-13  きむ茶工房     ArduinoIDE1.0.1に対応して変更          *
*******************************************************************************/
#include <Wire.h>
#include "arduino.h"
#include "skRTClib.h"

#define WRITE_FIX

byte skRTClib::Ctrl2 ;                  // Control2(Reg01)の設定内容を保存
byte skRTClib::InterFlag ;              // 外部割込みの有無をチェックするフラグ


skRTClib skRTC = skRTClib() ;             // Preinstantiate Objects

////////////////////////////////////////////////////////////////////////////////
// RTC(CLKOUT)からの外部割込みで処理される関数
////////////////////////////////////////////////////////////////////////////////
//void InterRTC()
//{
//     RTC.InterFlag = 1 ;
//}


// Constructors
skRTClib::skRTClib()
{
}
// ＢＣＤをバイナリ(10進数)に変換する処理(BCDは２桁まで対応)
unsigned int skRTClib::bcd2bin(byte dt)
{
     return ((dt >> 4) * 10) + (dt & 0xf) ;
}
// バイナリ(10進数)をＢＣＤに変換する処理(0-255までの変換に対応)
unsigned int skRTClib::bin2bcd(unsigned int num) 
{
     return ((num/100) << 8) | ( ((num%100) / 10) << 4 ) | (num%10) ;
}


/* int skRTC::start(byte Pin)
{
     Wire.begin() ;                     // Ｉ２Ｃの初期化、マスターとする
     delay(1000) ;                      // 1秒後に開始(RTC水晶振動子の発振を待つ)
} */


/*********************************************************************************
*  ans = begin(Pin,IntNum,InterRTC,Year,Mon,mDay,wDay,Hour,Min,Sec)              *
*    ＲＴＣの初期化を行う処理                                                    *
*    アラーム機能無効(OFF)で初期化しています。                                   *
*    タイマー機能は無効(OFF)、/INT端子出力は有・割込みは繰返すで初期化           *
*    クロック出力(CLKOUT)は1Hz(１秒間に１回ON)で初期化しています。               *
*                                                                                *
*    Pin    : 外部割込みを行うピンを指定    （CLKOUTは常に1Hzで出力している）    *
*             (0:割込みは無し　その他 : Pinの値のピン番号の端子で割込み)         *
*    IntNum : 割込み番号                                                         *
*    IntRTC : 割込み時のコールバック関数                                         *
*    Year   : 日付の年(0-99)を指定する(西暦の下２ケタ 2000年-2099年)             *
*    Mon    : 日付の月(1-12)を指定する                                           *
*    mDay   : 日付の日(1-31)を指定する(在りえない日を指定したら動作が不定らしい) *
*    wDay   : 曜日の指定(日[0] 月[1] 火[2] 水[3] 木[4] 金[5] 土[6])              *
*    Hour   : 時刻の時(0-23)を指定する                                           *
*    Min    : 時刻の分(0-59)を指定する                                           *
*    Sec    : 時刻の秒(0-59)を指定する                                           *
*    ans    : 戻り値、0=正常終了　それ以外I2C通信エラー下記                      *
*                     1=送ろうとしたデータが送信バッファのサイズを超えた         *
*                     2=スレーブ・アドレスを送信し、NACKを受信した               *
*                     3=データ・バイトを送信し、NACKを受信した                   *
*                     4=その他のエラー                                           *
*                     5=データ受信エラー                                         *
*********************************************************************************/
int skRTClib::begin(byte Pin,byte IntNum,void (*IntRTC)(void),byte Year,byte Mon,byte mDay,byte wDay,byte Hour,byte Min,byte Sec)
{
     int ans ;
     byte reg1 , reg2 ;

     // RTCからの外部割込みで処理する関数の登録を行う。
     InterFlag = 0 ;
     if (Pin > 0) {
          pinMode(Pin,INPUT) ;
          digitalWrite(Pin, HIGH) ;
          //attachInterrupt(Inter-2,InterRTC,RISING);
          attachInterrupt(IntNum,IntRTC,RISING);
     }
     // RTCの初期化を行うが、既に初期化済み(VL=0)なら初期化しない。
     Wire.begin() ;                     // Ｉ２Ｃの初期化、マスターとする
     delay(1000) ;                      // 1秒後に開始(RTC水晶振動子の発振を待つ)
     Wire.beginTransmission(RTC_ADRS) ; // 通信の開始処理（初期化状態のチェック）
#ifndef WRITE_FIX
     Wire.write(0x01) ;                 // レジスターアドレスは01hを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t) 0x01) ;                 // レジスターアドレスは01hを指定する
#endif /* WRITE_FIX */
     ans = Wire.endTransmission() ;     // データの送信と終了処理
     if (ans == 0) {
          ans = Wire.requestFrom(RTC_ADRS,2) ; // ＲＴＣにデータ送信要求をだす
          if (ans == 2) {
               reg1 = Wire.read()  ; // Reg 01H を受信する
               reg2 = Wire.read()  ; // Reg 02H を受信する
               if (reg2 & 0x80) {       // VLビットがＯＮなら初期化する
                    Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理（設定の書き込み）
#ifndef WRITE_FIX
                    Wire.write(0x00) ;                  // レジスターアドレスは00hを指定する
                    Wire.write(0x20) ;                  // Control1(Reg00)の設定(TEST=0,STOP=1)
                    Ctrl2 = 0x11    ;                   // 
                    Wire.write(Ctrl2);                  // Control2(Reg01)の設定(割込み禁止)
                    Wire.write((byte)bin2bcd(Sec)) ;    // Seconds(Reg02)の設定(時刻の秒0-59,VL=0)
                    Wire.write((byte)bin2bcd(Min)) ;    // Minutes(Reg03)の設定(時刻の分0-59)
                    Wire.write((byte)bin2bcd(Hour)) ;   // Hours(Reg04)の設定(時刻の時0-23)
                    Wire.write((byte)bin2bcd(mDay)) ;   // Days(Reg05)の設定(カレンダの日1-31)
                    Wire.write((byte)bin2bcd(wDay)) ;   // WeekDays(Reg06)の設定(カレンダの曜日0-6)
                    Wire.write((byte)bin2bcd(Mon)) ;    // Months(Reg07)の設定(カレンダの月1-12)
                    Wire.write((byte)bin2bcd(Year)) ;   // Years(Reg08)の設定(カレンダの年00-99)
                    Wire.write(0x80) ;                  // MinuteAlarm(Reg09)の設定(アラームの分無効)
                    Wire.write(0x80) ;                  // HourAlarm(Reg0A)の設定(アラームの時無効)
                    Wire.write(0x80) ;                  // HourAlarm(Reg0B)の設定(アラームの日無効)
                    Wire.write(0x80) ;                  // WeekDayAlarm(Reg0C)の設定(アラームの曜日無効)
                    Wire.write(0x83) ;                  // CLKOUT(Reg0D)の設定(1Hzで出力する)
                    Wire.write(0x00) ;                  // TimerControl(Reg0E)の設定(タイマ機能は無効)
                    Wire.write(0x00) ;                  // Timer(Reg0F)の設定(タイマ初期値は０)
#else /* WRITE_FIX */
                    Wire.write((uint8_t)0x00) ;                  // レジスターアドレスは00hを指定する
                    Wire.write((uint8_t)0x20) ;                  // Control1(Reg00)の設定(TEST=0,STOP=1)
                    Ctrl2 = 0x11    ;                   // 
                    Wire.write(Ctrl2);                  // Control2(Reg01)の設定(割込み禁止)
                    Wire.write((byte)bin2bcd(Sec)) ;    // Seconds(Reg02)の設定(時刻の秒0-59,VL=0)
                    Wire.write((byte)bin2bcd(Min)) ;    // Minutes(Reg03)の設定(時刻の分0-59)
                    Wire.write((byte)bin2bcd(Hour)) ;   // Hours(Reg04)の設定(時刻の時0-23)
                    Wire.write((byte)bin2bcd(mDay)) ;   // Days(Reg05)の設定(カレンダの日1-31)
                    Wire.write((byte)bin2bcd(wDay)) ;   // WeekDays(Reg06)の設定(カレンダの曜日0-6)
                    Wire.write((byte)bin2bcd(Mon)) ;    // Months(Reg07)の設定(カレンダの月1-12)
                    Wire.write((byte)bin2bcd(Year)) ;   // Years(Reg08)の設定(カレンダの年00-99)
                    Wire.write((uint8_t)0x80) ;                  // MinuteAlarm(Reg09)の設定(アラームの分無効)
                    Wire.write((uint8_t)0x80) ;                  // HourAlarm(Reg0A)の設定(アラームの時無効)
                    Wire.write((uint8_t)0x80) ;                  // HourAlarm(Reg0B)の設定(アラームの日無効)
                    Wire.write((uint8_t)0x80) ;                  // WeekDayAlarm(Reg0C)の設定(アラームの曜日無効)
                    Wire.write((uint8_t)0x83) ;                  // CLKOUT(Reg0D)の設定(1Hzで出力する)
                    Wire.write((uint8_t)0x00) ;                  // TimerControl(Reg0E)の設定(タイマ機能は無効)
                    Wire.write((uint8_t)0x00) ;                  // Timer(Reg0F)の設定(タイマ初期値は０)
#endif /* WRITE_FIX */
                    ans = Wire.endTransmission() ;      // データの送信と終了処理
                    if (ans == 0) {
                         Wire.beginTransmission(RTC_ADRS) ; // 通信の開始処理（時刻のカウント開始）
#ifndef WRITE_FIX
                         Wire.write(0x00) ;                 // レジスターアドレスは00hを指定する
                         Wire.write(0x00) ;                 // Control1(Reg00)の設定(TEST=0,STOP=0)
#else /* WRITE_FIX */
                         Wire.write((uint8_t)0x00) ;                 // レジスターアドレスは00hを指定する
                         Wire.write((uint8_t)0x00) ;                 // Control1(Reg00)の設定(TEST=0,STOP=0)
#endif /* WRITE_FIX */
                         ans = Wire.endTransmission() ;     // データの送信と終了処理
                         delay(1000) ;                      // カウント開始を待つ
                    }
               } else {
                    Ctrl2 = reg1 ;
                    ans = 0 ;
               }
          } else ans = 5 ;
     }
     return ans ;
}
/*******************************************************************************
*  ans = sTime(Year,Mon,mDay,wDay,Hour,Min,Sec)                                *
*    ＲＴＣに日付と時刻を書き込む処理                                          *
*                                                                              *
*    Year : 日付の年(0-99)を指定する(西暦の下２ケタ 2000年-2099年)             *
*    Mon  : 日付の月(1-12)を指定する                                           *
*    mDay : 日付の日(1-31)を指定する(在りえない日を指定したら動作が不定らしい) *
*    wDay : 曜日の指定(日[0] 月[1] 火[2] 水[3] 木[4] 金[5] 土[6])              *
*    Hour : 時刻の時(0-23)を指定する                                           *
*    Min  : 時刻の分(0-59)を指定する                                           *
*    Sec  : 時刻の秒(0-59)を指定する                                           *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::sTime(byte Year,byte Mon,byte mDay,byte wDay,byte Hour,byte Min,byte Sec)
{
     int ans ;

     Wire.beginTransmission(RTC_ADRS) ;       // 通信の開始処理（時刻の設定）
#ifndef WRITE_FIX
     Wire.write(0x00) ;                       // レジスターアドレスは00hを指定する
     Wire.write(0x20) ;                       // Control1(Reg00)の設定(TEST=0,STOP=1)
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x00) ;                       // レジスターアドレスは00hを指定する
     Wire.write((uint8_t)0x20) ;                       // Control1(Reg00)の設定(TEST=0,STOP=1)
#endif /* WRITE_FIX */
     ans = Wire.endTransmission() ;           // データの送信と終了処理
     if (ans == 0) {
          Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理（時刻の設定）
#ifndef WRITE_FIX
          Wire.write(0x02) ;                  // レジスターアドレスは02hを指定する
#else /* WRITE_FIX */
          Wire.write((uint8_t)0x02) ;                  // レジスターアドレスは02hを指定する
#endif /* WRITE_FIX */
          Wire.write((byte)bin2bcd(Sec)) ;    // Seconds(Reg02)の設定(時刻の秒0-59,VL=0)
          Wire.write((byte)bin2bcd(Min)) ;    // Minutes(Reg03)の設定(時刻の分0-59)
          Wire.write((byte)bin2bcd(Hour)) ;   // Hours(Reg04)の設定(時刻の時0-23)
          Wire.write((byte)bin2bcd(mDay)) ;   // Days(Reg05)の設定(カレンダの日1-31)
          Wire.write((byte)bin2bcd(wDay)) ;   // WeekDays(Reg06)の設定(カレンダの曜日0-6)
          Wire.write((byte)bin2bcd(Mon)) ;    // Months(Reg07)の設定(カレンダの月1-12)
          Wire.write((byte)bin2bcd(Year)) ;   // Years(Reg08)の設定(カレンダの年00-99)
          ans = Wire.endTransmission() ;      // データの送信と終了処理
          if (ans == 0) {
               Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理（時刻のカウント開始）
#ifndef WRITE_FIX
               Wire.write(0x00) ;                  // レジスターアドレスは00hを指定する
               Wire.write(0x00) ;                  // Control1(Reg00)の設定(TEST=0,STOP=0)
#else /* WRITE_FIX */
               Wire.write((uint8_t)0x00) ;                  // レジスターアドレスは00hを指定する
               Wire.write((uint8_t)0x00) ;                  // Control1(Reg00)の設定(TEST=0,STOP=0)
#endif /* WRITE_FIX */
               ans = Wire.endTransmission() ;      // データの送信と終了処理
               delay(1000) ;                       // カウント開始を待つ
          }
     }
     return ans ;
}
/*******************************************************************************
*  ans = rTime(*tm)                                                            *
*    ＲＴＣから現在の日付と時刻を読み取る処理                                  *
*                                                                              *
*    *tm  : 読み取ったデータを保存する配列変数を指定する(配列は7バイト必要)    *
*           配列にはＢＣＤデータで秒・分・時・日・曜日・月・年の順で保存する   *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::rTime(byte *tm)
{
     int i , ans ;

     Wire.beginTransmission(RTC_ADRS) ;        // 通信の開始処理（時刻の受信）
#ifndef WRITE_FIX
     Wire.write(0x02) ;                        // レジスターアドレスは02hを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x02) ;                        // レジスターアドレスは02hを指定する
#endif /* WRITE_FIX */
     ans = Wire.endTransmission() ;            // データの送信と終了処理
     if (ans == 0) {
          ans = Wire.requestFrom(RTC_ADRS,7) ; // ＲＴＣにデータ送信要求をだす
          if (ans == 7) {
               *tm = Wire.read() & 0x7f ;   // 秒
               tm++ ;
               *tm = Wire.read() & 0x7f ;   // 分
               tm++ ;
               *tm = Wire.read() & 0x3f ;   // 時
               tm++ ;
               *tm = Wire.read() & 0x3f ;   // 日
               tm++ ;
               *tm = Wire.read() & 0x07 ;   // 曜日
               tm++ ;
               *tm = Wire.read() & 0x1f ;   // 月
               tm++ ;
               *tm = Wire.read() ;          // 年
               ans = 0 ;
          } else ans = 5 ;
     }
     return ans ;
}
/*******************************************************************************
*  cTime(*tm,*c)                                                               *
*    ＲＴＣの日付と時刻を文字列に変換する処理                                  *
*                                                                              *
*    *tm  : rTimeから読み込んだ配列変数を指定する                              *
*    *c   : 文字列に変換したデータ(24バイト)を格納する配列変数を指定する       *
*           "yyyy/mm/dd www hh:mm:ss"で返す　例)"2010/01/15 TUE 15:30:00"      *
*******************************************************************************/
void skRTClib::cTime(byte *tm, byte *c)
{
     char dt[7][4] = { "SUN","MON","TUE","WED","THU","FRI","SAT" } ;
     byte buf[24] ;

     buf[23] = 0x00 ;
     set_ctime(*tm,':',&buf[20]) ;    // 秒
     tm++ ;
     set_ctime(*tm,':',&buf[17]) ;    // 分
     tm++ ;
     set_ctime(*tm,' ',&buf[14]) ;    // 時
     tm++ ;
     buf[10] = ' ' ;
     set_ctime(*tm,'/',&buf[7]) ;    // 日
     tm++ ;
     memcpy(&buf[11],&dt[*tm & 0x0f][0],3) ; // 曜日
     tm++ ;
     set_ctime(*tm,'/',&buf[4]) ;    // 月
     tm++ ;
     set_ctime(*tm,'0',&buf[1]) ;    // 年
     buf[0] = '2' ;
     memcpy(c,buf,24) ;
}
// 日付と時刻を文字列に変換する処理のサブ関数
void skRTClib::set_ctime(byte tm,byte s,byte *a)
{
     *a = s ;
     a++ ;
     *a = (tm >> 4) + 0x30 ;
     a++ ;
     *a = (tm & 0x0f) + 0x30 ;
}
/*******************************************************************************
*  ans = SetTimer(sclk,count)                                                  *
*    定周期タイマーの設定をする処理（タイマーの開始）                          *
*                                                                              *
*    sclk : ソースクロックの指定(244.14us[0] 15.625ms[1] 1sec[2] 1min[3])      *
*    count: カウント値の指定(1-255) sclk=2 count=10 なら10秒周期となる         *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::SetTimer(byte sclk,byte count)
{
     int ans ;

     Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理
#ifndef WRITE_FIX
     Wire.write(0x0f) ;                  // レジスターアドレスは0fhを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x0f) ;                  // レジスターアドレスは0fhを指定する
#endif /* WRITE_FIX */
     Wire.write(count);                  // カウントダウンタイマー値の設定
     ans = Wire.endTransmission() ;      // データの送信と終了処理
     Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理
#ifndef WRITE_FIX
     Wire.write(0x0e) ;                  // レジスターアドレスは0ehを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x0e) ;                  // レジスターアドレスは0ehを指定する
#endif /* WRITE_FIX */
     Wire.write(sclk | 0x80) ;           // ソースクロックの設定とタイマーの開始
     ans = Wire.endTransmission() ;      // データの送信と終了処理
     return ans ;
}
/*******************************************************************************
*  ans = StopTimer()                                                           *
*    定周期タイマー機能を無効にする処理（タイマーの終了）                      *
*                                                                              *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::StopTimer()
{
     int ans ;

     Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理
#ifndef WRITE_FIX
     Wire.write(0x0e) ;                  // レジスターアドレスは0ehを指定する
     Wire.write(0x00) ;                  // タイマーの終了
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x0e) ;                  // レジスターアドレスは0ehを指定する
     Wire.write((uint8_t)0x00) ;                  // タイマーの終了
#endif /* WRITE_FIX */
     ans = Wire.endTransmission() ;      // データの送信と終了処理
     Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理（アラームの開始）
#ifndef WRITE_FIX
     Wire.write(0x01) ;                  // レジスターアドレスは01hを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x01) ;                  // レジスターアドレスは01hを指定する
#endif /* WRITE_FIX */
     Ctrl2 = Ctrl2 & 0xfb ;              // タイマーフラグをクリア(TF=0)
     Wire.write(Ctrl2);                  // Control2(Reg01)の設定
     ans = Wire.endTransmission() ;      // データの送信と終了処理
     return ans ;
}
/*******************************************************************************
*  ans = SetAlarm(Hour,Min,mDay,wDay)                                          *
*    アラームの日時を設定する処理（アラームの開始）                            *
*                                                                              *
*    Hour : 時刻の時(0-23)を指定する、0xff指定でHour設定は無効                 *
*    Min  : 時刻の分(0-59)を指定する、0xff指定でMin設定は無効                  *
*    mDay : 日付の日(1-31)を指定する、0xff指定でmDay設定は無効                 *
*    wDay : 曜日の指定(日[0] 月[1] 火[2] 水[3] 木[4] 金[5] 土[6])              *
*           0xff指定でwDay設定は無効                                           *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::SetAlarm(byte Hour,byte Min,byte mDay,byte wDay)
{
     int ans ;

     Wire.beginTransmission(RTC_ADRS) ;    // 通信の開始処理（アラーム時刻の設定）
#ifndef WRITE_FIX
     Wire.write(0x09) ;                    // レジスターアドレスは09hを指定する
     if (Min == 0xff)  Wire.write(0x80)   ;// MinuteAlarm(Reg09)の設定(分は無効)
     else Wire.write((byte)bin2bcd(Min))  ;// 0-59(分は有効)
     if (Hour == 0xff) Wire.write(0x80)   ;// HoursAlarm(Reg0A)の設定(時は無効)
     else Wire.write((byte)bin2bcd(Hour)) ;// 0-23(時は有効)
     if (mDay == 0xff) Wire.write(0x80)   ;// DaysAlarm(Reg0B)の設定(日は無効)
     else Wire.write((byte)bin2bcd(mDay)) ;// 1-31(日は有効)
     if (wDay == 0xff) Wire.write(0x80)   ;// WeekDaysAlarm(Reg0C)の設定(曜日は無効)
     else Wire.write((byte)bin2bcd(wDay)) ;// 0-6(曜日は有効)
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x09) ;                    // レジスターアドレスは09hを指定する
     if (Min == 0xff)  Wire.write((uint8_t)0x80)   ;// MinuteAlarm(Reg09)の設定(分は無効)
     else Wire.write((byte)bin2bcd(Min))  ;// 0-59(分は有効)
     if (Hour == 0xff) Wire.write((uint8_t)0x80)   ;// HoursAlarm(Reg0A)の設定(時は無効)
     else Wire.write((byte)bin2bcd(Hour)) ;// 0-23(時は有効)
     if (mDay == 0xff) Wire.write((uint8_t)0x80)   ;// DaysAlarm(Reg0B)の設定(日は無効)
     else Wire.write((byte)bin2bcd(mDay)) ;// 1-31(日は有効)
     if (wDay == 0xff) Wire.write((uint8_t)0x80)   ;// WeekDaysAlarm(Reg0C)の設定(曜日は無効)
     else Wire.write((byte)bin2bcd(wDay)) ;// 0-6(曜日は有効)
#endif /* WRITE_FIX */
     ans = Wire.endTransmission() ;        // データの送信と終了処理
     Wire.beginTransmission(RTC_ADRS) ;    // 通信の開始処理（アラームの開始）
#ifndef WRITE_FIX
     Wire.write(0x01) ;                    // レジスターアドレスは01hを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x01) ;                    // レジスターアドレスは01hを指定する
#endif /* WRITE_FIX */
     Ctrl2 = (Ctrl2 | 0x02) & 0xf7 ;       // アラームを有効にする(AIE=1 AF=0)
     Wire.write(Ctrl2);                    // Control2(Reg01)の設定(割込み)
     ans = Wire.endTransmission() ;        // データの送信と終了処理
     return ans ;
}
/*******************************************************************************
*  ans = StopAlarm()                                                           *
*    アラーム機能を無効にする処理（アラーム終了）                              *
*                                                                              *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::StopAlarm()
{
     int ans ;

     Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理（アラームの停止）
#ifndef WRITE_FIX
     Wire.write(0x01) ;                  // レジスターアドレスは01hを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x01) ;                  // レジスターアドレスは01hを指定する
#endif /* WRITE_FIX */
     Ctrl2 = Ctrl2 & 0xf5 ;              // アラームを無効にする(AIE=0 AF=0)
     Wire.write(Ctrl2);                  // Control2(Reg01)の設定(割込み)
     ans = Wire.endTransmission() ;      // データの送信と終了処理
     return ans ;
}
/*******************************************************************************
*  ans = offAlarm()                                                            *
*    アラームの発生を解除する処理（アラームは引き続き有効）                    *
*                                                                              *
*    ans  : 戻り値、0=正常終了　それ以外I2C通信エラー(begin関数を参照)         *
*******************************************************************************/
int skRTClib::offAlarm()
{
     int ans ;

     Wire.beginTransmission(RTC_ADRS) ;  // 通信の開始処理（アラーム出力の解除）
#ifndef WRITE_FIX
     Wire.write(0x01) ;                  // レジスターアドレスは01hを指定する
#else /* WRITE_FIX */
     Wire.write((uint8_t)0x01) ;                  // レジスターアドレスは01hを指定する
#endif /* WRITE_FIX */
     Ctrl2 = Ctrl2 & 0xf7 ;              // アラームの出力を解除する(AF=0)
     Wire.write(Ctrl2);                  // Control2(Reg01)の設定(割込み)
     ans = Wire.endTransmission() ;      // データの送信と終了処理
     return ans ;
}
