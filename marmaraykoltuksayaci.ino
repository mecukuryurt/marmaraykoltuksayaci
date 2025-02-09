// mecukuryurt tarafindan 09.02.2025 tarihinde yazildi.

// https://github.com/prenticedavid/MCUFRIEND_kbv
// Kutuphaneler ice aktarilir.
#include "Adafruit_GFX.h"
#include "MCUFRIEND_kbv.h"
#include <SoftwareSerial.h>

// Bluetooth ve LCD ekran objeleri tanimlanir
SoftwareSerial BT(10, 11); 
MCUFRIEND_kbv tft;

// Alinacak veriyi tutacak degiskenler tanimlanir.
const byte numChars = 30; // max string uzunlugu
char receivedChars[numChars]; // alinan veri
char boardingStation[numChars]; // binilen istasyon
char arrivalStation[numChars];  // inilecek istasyon
boolean newData = false, debug=true, isConnected=false;
// Yeni bilgi geldi mi, cikti verilsin mi, baglanti var mi

// Renk kodlari
const uint16_t RED     = 0xF800;
const uint16_t GREEN   = 0x0620;
const uint16_t BLUE    = 0x001F;
const uint16_t CYAN    = 0x07FF;
const uint16_t MAGENTA = 0xF81F;
const uint16_t YELLOW  = 0xFD60;
const uint16_t WHITE   = 0xFFFF;
const uint16_t GRAY    = 0x520A;
const uint16_t BLACK   = 0x0000;

// Girdinin alabilecegi iki kalip deger.
// Karsilastirma icin.
const char statusok[] = "STATUS OK";
const char station[]  = "STATION";

// istasyon sayisi, kalan istasyon sayisi
int stationCount=0, remainingStationCount=0;
uint16_t currentColor = RED, textColor=WHITE, lbsc=WHITE, lbic=CYAN;
// anlik arkaplan rengi, yazi rengi, ilerleme cubugu kenarlik rengi, ic rengi
 
void setup() {
    // arka plan rengini yesil yap
    currentColor = GREEN;
    Serial.begin(115200); // Serial ekrana yazmayi baslat
    BT.begin(9600);       // Bluetooth haberlesmeyi ac
    tft.begin(0x9486); // ekran baslatilir (3.5 inc ekran icin 0x9486)

    // Bombos bir ilerleme cubugu ciz ve yazilari yaz
    drawLoadingBar(150, 10, 10, 75, currentColor, lbsc, lbic, 20, 0, 5);
    displayText(currentColor, textColor, 20, 50, "\0", "\0", 20);
    Serial.println(isConnected);
    if (!isConnected) { // Bagli degil isek
      BT.println("<STATUS INIT>"); // Cihaza baslatildigina dair bilgi ver
      Serial.println("<STATUS INIT>");
  
      while (true) {
        if (BT.available() > 0)     {  recvWithStartEndMarkers(); } // Bilgi geliyor ise kaydet
        if (newData) { // Eger yeni bilgi geliyor ise
          Serial.print("DATA:"); // bilgiyi yazdir
          Serial.println(receivedChars);  
          newData=false;
          if (receivedChars == "STATUS OK") {Serial.println("OK");
          // Serial.println(compareStrings(statusok, receivedChars));
          if (compareStrings(statusok, receivedChars)) {Serial.println("OK"); break;} // Basarili baglanti
        }
      }
    }
    isConnected=true;

    BT.println("<GET BOARDINGSTATIONNAME>"); // binilen istasyonu iste
    Serial.println("<GET BOARDINGSTATIONNAME>");
    while (true) {
      if (BT.available() > 0)     {  recvWithStartEndMarkers(); } // bilgi geliyor ise kaydet
      if (newData) {
        Serial.print("DATA:");
        Serial.println(receivedChars);  
        newData=false;
        if (compareStrings(statusok, receivedChars)) {setup(); return;} // Baslangictaki talimat geliyor ise resetle
        
        for (int a=0;receivedChars[a];a++) boardingStation[a]=receivedChars[a]; // Binis Istasyonu degiskenine ata
        // boardingStation = receivedChars;
        break;
      }
    }
    BT.println("<GET ARRIVALSTATIONNAME>"); // inilecek istasyonu iste
    Serial.println("<GET ARRIVALSTATIONNAME>");
    while (true) {
      if (BT.available() > 0)     {  recvWithStartEndMarkers(); } // bilgi gelyior ise kaydet
      if (newData) {
        Serial.print("DATA:");
        Serial.println(receivedChars);  
        newData=false;
        if (compareStrings(statusok, receivedChars)) {setup(); return;} // Baslangictaki talimat ise resetle
        for (int b=0;receivedChars[b];b++) arrivalStation[b]=receivedChars[b]; // Inis istasyonuna ata
        // arrivalStation = receivedChars;
        break;
      }
    }
    BT.println("<GET STATIONCOUNT>"); // Aradaki istasyon sayisini iste
    Serial.println("<GET STATIONCOUNT>");
    while (true) {
      if (BT.available() > 0)     {  recvWithStartEndMarkers(); } // bilgi geliyor ise kaydet
      if (newData) {
        Serial.print("DATA:");
        Serial.println(receivedChars);  
        newData=false;
        if (compareStrings(statusok, receivedChars)) {setup(); return;} // baslangic talimati geldiyse resetle
        remainingStationCount = stationCount = atoi(receivedChars); // char degerini integer yap
        Serial.print("STATIONCOUNT: ");Serial.println(stationCount);
        // arrivalStation = receivedChars;
        break;
      }
    }

    currentColor=RED; // Arkaplani kirmizi yap
    // int barX, int barY, int barStroke, int barW, uint16_t screenColour, uint16_t barColour, uint16_t pieceColour, int pieceCount, int progress, int piecePadding
    drawLoadingBar(150, 10, 10, 75, currentColor, lbsc, lbic, stationCount, 0, 5);
    // uint16_t bg, uint16_t tfg, int textX, int textY, char* as, char* bs, int remaining
    displayText(currentColor, textColor, 20, 50, arrivalStation, boardingStation, stationCount);
}

// Iki string esit mi degil mi kontrol et
bool compareStrings(char* str1, char* str2) {
  bool same=true;
  for (int i=0; str1[i] && str2[i]; i++) {
   if (str1[i] != str2[i]) {
    same = false;
    break; 
   }
  }
  return same;
}

void loop() 
{    
    if (BT.available() > 0)     {  recvWithStartEndMarkers(); } // veri geliyor mu
    if (newData) {
      Serial.print("DATA:");
      Serial.println(receivedChars);
      newData=false;

      if (compareStrings(station, receivedChars)) { // eger "STATION" talimati geldiyse
        while (true) { // Kalan istasyon sayisini iste
          if (BT.available() > 0)     {  recvWithStartEndMarkers(); }
          if (newData) {
            Serial.print("REMS:");
            Serial.println(receivedChars);  
            if (compareStrings(statusok, receivedChars)) {Serial.println("CONNECTION RESET"); setup(); return;}
            newData=false;
            remainingStationCount = atoi(receivedChars); // veriyi integer'a cevir
            // arrivalStation = receivedChars;
            break;
          }
        }
        if (remainingStationCount <= 0) {delay(3000); setup();} // eger 0 istasyon kaldiysa vardik demektir. resetle
        else if (remainingStationCount <= 5) currentColor = GREEN; // 5 duraktan az kaldi ise yesile cevir
        else if (remainingStationCount <= 15) currentColor = YELLOW; // 15 duraktan az kaldi ise sariya degistir.
        else currentColor = RED; // daha fazla ise kirmizi yap

        // kalan durak degerini vererek ilerleme çubuğunu olustur.
        drawLoadingBar(150, 10, 10, 75, currentColor, lbsc, lbic, stationCount, stationCount-remainingStationCount, 5);
        displayText(currentColor, textColor, 20, 50, arrivalStation, boardingStation, remainingStationCount);
        // Durak bilgisini ve kalan duragi yazdir.
      }
      if (compareStrings(statusok, receivedChars)) {Serial.println("CONNECTION RESET"); setup();} // Baslangic talimati geldi ise resetle.
    }
}     

void recvWithStartEndMarkers(){
     // function recvWithStartEndMarkers by Robin2 of the Arduino forums
     // See  http://forum.arduino.cc/index.php?topic=288234.0

     // bu fonksiyon cagrildiginda bluetooth stream'inden gönderilen bilginin
     // < ve > karakterleri arasindaki bolumunu alip receivedChars degiskenine atar.
 
     static boolean recvInProgress = false;
     static byte ndx = 0;
     char startMarker = '<';
     char endMarker = '>';
     char rc;
 
     if (BT.available() > 0) {
          rc = BT.read();
          if (recvInProgress == true) {
               if (rc != endMarker) {
                    receivedChars[ndx] = rc;
                    ndx++;
                    if (ndx >= numChars) { ndx = numChars - 1; }
               }
               else {
                     receivedChars[ndx] = '\0'; // terminate the string
                     recvInProgress = false;
                     ndx = 0;
                     newData = true;
               }
          }
          else if (rc == startMarker) { recvInProgress = true; }
     }
}


// Durak bilgilerini yazdir.
void displayText(uint16_t bg, uint16_t tfg, int textX, int textY, char* as, char* bs, int remaining) {
  tft.setTextColor(tfg, bg);
    tft.setTextSize(3);
    tft.setCursor(textX, textY);
    tft.setRotation(3);
    tft.print("Binis:");
    tft.print(bs);
    tft.setCursor(textX, textY+40);
    tft.print("Inis :");
    tft.print(as);
    tft.setCursor(textX, textY+220);
    tft.print("Kalan Durak: ");
    tft.print(remaining);
}


// Ilerleme cubugunu ciz.
void drawLoadingBar(int barX, int barY, int barStroke, int barW, uint16_t screenColour, uint16_t barColour, uint16_t pieceColour, int pieceCount, int progress, int piecePadding) {
  tft.setRotation(0);
    // barX ve barY degiskenleri, cubugun sol ust kosesinin
    // piksel bazinda koordinatlarini belirtir.
    // barStroke degiskeni, cubugun ici ile disi arasindaki
    // mesafeyi belirtir.
    // const int barX=150, barY=10, barStroke=10;
    // barH ve barW degiskenleri, cubugun yukseklik ve genisligini belirtir
    const int barH=tft.height()-2*barY; // barW=75;

    // Ekran tamamen beyaz renk ile doldurulur
    tft.fillScreen(screenColour);
    // Cubuk kismi kirmizi ile doldurulur
    tft.fillRect(barX, barY, barW, barH, barColour);
    // Cubugun ic kismi bosaltilir (beyaz ile doldurulur)
    tft.fillRect(barX+barStroke, barY+barStroke, barW-2*barStroke, barH-2*barStroke, screenColour);

    // pieceCount = Aradaki kutucuk sayisi (kullanicinin gidecegi toplam durak sayisi)
    // piecePadding = Kutucuklar ve cubugun ic kismi arasindaki mesafe
    // innerHeight = Cubugun ic kisminin yuksekligi
    const int innerHeight=barH-2*barStroke; // pieceCount=10, piecePadding=5, 
    // pieceWidth = Her bir kutucugun genisligi
    // pieceHeight = Her bir kutucugun yuksekligi
    const int pieceWidth = (innerHeight-piecePadding) / pieceCount-piecePadding, pieceHeight = barW-2*(barStroke+piecePadding);
    // positionX = Her bir kutucugun sol kisminin dayali olacagi apsis degeri
    const int positionX = barX+barStroke+piecePadding;

    // pieceWidth hesaplanirken innerHeight-piecePadding degerinin pieceCount degerine tam bolunmemesi durumunda 
    // bolme isleminden kalan deger, blankSpace degiskenine aktarilir. Bu degiskenin ilk degeri initialBlankSpace degiskenine atanir
    int blankSpace;
    int initialBlankSpace = blankSpace = (innerHeight-piecePadding) % pieceCount;
    // Her bir kutucuk icin
    for (int piece=0; piece<progress; piece++) {
      // blankSpace eksiye duser ise 0 yap
      blankSpace = (blankSpace < 0)?0:blankSpace;
      // Kutucugun alt kenarinin dayali olacagi ordinat degeri. Her bir kutucuk icin farkli deger alacak.
      //              En asagiya in, bir padding yukari cik.     Her bir kutucuk icin yukari cik.
      int positionY = barY+barStroke+innerHeight -piecePadding - piece*(pieceWidth+piecePadding) - (initialBlankSpace-blankSpace--); // - pieceWidth;
      tft.fillRect(positionX, positionY, pieceHeight, -pieceWidth - (blankSpace >= 0), pieceColour);
      // delay(1000);
    }
}
