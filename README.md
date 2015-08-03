# skRTClib
RTC-8564NBをarduinoから使うためのライブラリ

# 開発の経緯
元々は[きむ茶さんのRTCライブラリ][skRTClibOrig]を利用しようとしたが，使ってみると
幾つか問題があったこと，また，Arduino M0では変数名がかちあっていて
ライブラリがコンパイルエラーとなることなど面倒を避けるために改修した．

# 修正箇所
* Arduino M0 proの開発環境では，大文字(RTC)はすでに使われているため，ライブラリ内部で定義しているクラスのオブジェクトがエラーとなる．
* RTCからの外部割り込み発生時に実行される割り込み関数はアプリ開発者が定義すべきだと思うので，ライブラリからは排除した．
* 外部割り込みの番号やピン配置に「機種依存の想定」を置いており，Mega系列では動作しない．

# 未修正の問題
* [きむ茶さんのRTCライブラリ][skRTClibOrig]に添付されているサンプルプログラムは，古いバージョンのライブラリをベースにしているのか，現在配布されている[きむ茶さんのRTCライブラリ][skRTClibOrig]とは整合性がない．

# 将来計画
とりあえず，サンプルプログラムを修正したら，「きむ茶さん」に還元しようかと．
(引き取ってくれるかどうか不明だけどね～)

# 参考文献
ソースの先頭にも記述しているが，参考にした情報源は以下の通り．
* きむ茶さん <http://www.geocities.jp/zattouka/GarageHouse/micon/Arduino/RTC/RTC.htm>

<!--以下はリンクの定義-->
<!--参考文献-->
[kimcha]: <http://www.geocities.jp/zattouka/GarageHouse/micon/Arduino/RTC/RTC.htm> "きむ茶さん"

<!--開発環境と各種ライブラリ-->
[ide]: <http://www.arduino.cc/en/Main/Software> "Arduino開発環境"
[skRTClibOrig]: <http://www.geocities.jp/zattouka/GarageHouse/micon/Arduino/RTC/skRTClib.lzh> "きむ茶さんのRTCライブラリ"
[skRTClib]: <https://github.com/houtbrion/skRTClib> "きむ茶さんのRTCライブラリを改造したバージョン"

<!--ハード関連-->
[Uno]: <http://www.arduino.cc/en/Main/ArduinoBoardUno> "Arduino Uno"
[Mega2560]: <http://www.arduino.cc/en/Main/ArduinoBoardMega2560> "Arduino Mega 2560"
[M0pro]: <http://www.arduino.org/products/arduino-m0-pro> "Arduino M0 pro"
[case]: <https://www.sengoku.co.jp/mod/sgk\_cart/detail.php?code=EEHD-4CLA> "Arduino用ケース(千石電商)"
[rtc]: <http://akizukidenshi.com/catalog/g/gI-00233/> "3564NB(秋月電子通商)"


