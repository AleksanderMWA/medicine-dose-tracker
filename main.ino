// biblioteki WI-FI i czasu

#include <WiFi.h>
#include "time.h"
#include "esp_sntp.h"


// biblioteka do obsługi komunikacji za pomocą protokołu I2C

#include <Wire.h>


// biblioteki sterownika wyświetlacza

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>


// biblioteki od karty SD i zapisywania na nich plików .txt

#include "FS.h"
#include "SD.h"
#include "SPI.h"


// ustawienie baud rate do komunikacji z serial monitorem

const int baudRate = 115200;


// ustawienie wyswietlacza 

const int lcdColumns = 16;
const int lcdRows = 2;


// ustawienia sieci WI-FI

const char* ssid = "someSSID";
const char* password = "somePassword";


// ustawienia serwera NTP do pobrania czasu

const char *ntpServer1 = "ntp1.icm.edu.pl"; // Serwer NTP w Polsce
const char *ntpServer2 = "pl.pool.ntp.org"; // Alternatywny serwer NTP
const long gmtOffset_sec = 3600; // Różnica czasu od GMT (UTC+1)
const int daylightOffset_sec = 3600; // Dodatkowa godzina dla czasu letniego
const char *time_zone = "CET-1CEST,M3.5.0/2,M10.5.0/3"; // Strefa czasowa Polski


// ustawienia pinów GPIO przycisków

const int redButtonPin = 4;
const int yellowButtonPin = 17;
const int greenButtonPin = 27;


// funkcja wywolywana, gdy dostosuje się czas za pomocą NTP. Jeżeli połączenie się powiedzie, wówczas w Serial monitorze wyświetli wiadomość

void timeavailable(struct timeval *t) 
{
  Serial.println("Got time adjustment from NTP!");  
}


// funkcja do zwracania aktualnej daty RRRR-MM-DD w formie String

String getCurrentDate() 
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) 
  {
    return "Brak daty";  // w przypadku braku daty zwróć komunikat o błędzie
  }

  char buffer[11]; 
  strftime(buffer, sizeof(buffer), "%d-%m-%Y", &timeinfo);  // w przypadku pobrania daty zwróć datę
  return String(buffer);
}


// funkcja do zwracania aktualnej godziny HH-MM czasu Polskiego w formie String

String getCurrentTime() 
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) 
  {
    return "Brak czasu";  // w przypadku braku czasu zwróć komunikat o błędzie
  }

  char buffer[9]; 
  strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo); // w przypadku pobrania czasu zwróć czas
  return String(buffer);
}


// tworzenie nowego katalogu w systemie plików

void createDir(fs::FS &fs, const char *path) 
{
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) 
  {
    Serial.println("Dir created"); // w przypadku utworzenia katalogu, zwróć komunikat o sukcesie operacji
  } 
  
  else 
  {
    Serial.println("mkdir failed"); // w przypadku nie utworzenia katalogu, zwróć komunikat o błędzie
  }
}


// dodawanie danych na koniec pliku w systemie plików

void appendFile(fs::FS &fs, const char *path, const char *message) 
{
  Serial.printf("Dodawanie na koniec pliku: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  
  if (!file) 
  {
    Serial.println("Nie udało się otworzyć pliku do zapisu!");
    return;
  }
  
  if (file.print(message)) 
  {
    Serial.println("Zapisano datę na koniec pliku!");
  } 
  
  else 
  {
    Serial.println("Nie udało się zapisać daty na koniec pliku");
  }
  
  file.close();
}

// inicjalizacja wyświetlacza w formie globalnego obiektu, do którego łatwiej się potem odwoływać

hd44780_I2Cexp lcd;  


// zmienne przechowujące stany przycisków. 0 - nie wciśnięty, 1 - wciśnięty. domyślnie jest 0.

int redButtonState = 0;
int yellowButtonState = 0;
int greenButtonState = 0;


// nazwa pliku.txt wraz ze ścieżką na karcie SD

const char *fileName = "/lista.txt"; 


// podliczanie ilości linii w plku .txt. zwraca ilość linii pliku .txt

int countLines(const char *fileName)
{
  if (!SD.exists(fileName)) 
  {
    Serial.printf("Plik %s nie istnieje na karcie SD.\n", fileName); // w przypadku nie znalezienia pliku, wyświetl błąd w serial monitorze
    return 0;
  }

  File file = SD.open(fileName, FILE_READ);
  if (!file) 
  {
    Serial.println("Nie udało się otworzyć pliku do odczytu."); // w przypadku niepowodzenia w trakcie otwierania pliku, wyświetl błąd w serial monitorze
    return 0;
  }

  int lineCount = 0;

  while (file.available()) 
  {
    if (file.read() == '\n') 
    {
      lineCount++;
    }
  }

  file.close();

  return lineCount; 
}


// usuwanie ostatniej linii z pliku

void deleteLine(const char *fileName, int lineToDelete) 
{
  if (!SD.exists(fileName)) 
  {
    Serial.printf("Plik %s nie istnieje na karcie SD.\n", fileName); // w przypadku nie znalezienia pliku, wyświetl błąd w serial monitorze
    return;
  }

  File file = SD.open(fileName, FILE_READ);
  if (!file) 
  {
    Serial.println("Nie udało się otworzyć pliku do odczytu."); // w przypadku niepowodzenia w trakcie otwierania pliku, wyświetl błąd w serial monitorze
    return;
  }

  String fileContent = "";
  int currentLine = 1;
  int totalLines = 0;

  while (file.available()) 
  {
    String line = file.readStringUntil('\n');
    totalLines++;

    if (currentLine != lineToDelete) 
    {
      fileContent += line + '\n'; // dodawanie linii, jeśli nie jest usuwana
    }

    currentLine++;
  }
  file.close();


  // usuń końcowy znak '\n', jeśli pozostał pusty enter

  if (fileContent.endsWith("\n")) {
    fileContent = fileContent.substring(0, fileContent.length() - 1);
  }


  // jeśli plik ma tylko jedną linię i ją usuwamy, usuń całą zawartość

  if (totalLines == 1 && lineToDelete == 1) {
    fileContent = "";
  }

  // Zapisz zmodyfikowaną zawartość do pliku
  file = SD.open(fileName, FILE_WRITE);
  if (file)
  {
    file.print(fileContent);
    file.close();
    Serial.printf("Linia %d została usunięta. Plik teraz ma %d linie.\n", lineToDelete, totalLines - 1);
  } 
  
  else 
  {
    Serial.println("Nie udało się otworzyć pliku w trybie zapisu."); // w przypadku niepowodzenia w trakcie otwierania pliku, wyświetl błąd w serial monitorze
  }
}


// funkcja do jednorazowej inicjalizacji najważniejszych ustawień

void setup() 
{
  // inicjalizowanie połączenia między serial monitorem a ESP32

  Serial.begin(baudRate);


  // podłączanie się do sieci

  Serial.print("Podlaczam sie do sieci.");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" OK");


  // wywołanie funkcji informującej o połączeniu z serwerem NTP

  sntp_set_time_sync_notification_cb(timeavailable);


  // ustawianie czasu skonfigurowanego w oparciu o serwer NTP

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);


  // inicjalizowanie wyświetlacza

  lcd.begin(lcdColumns, lcdRows);
  

  // inicjalizowanie przycisków oraz pullup rezystorów

  pinMode(redButtonPin, INPUT_PULLUP);  
  pinMode(yellowButtonPin, INPUT_PULLUP); 
  pinMode(greenButtonPin, INPUT_PULLUP); 


  // testowanie karty SD

  if (!SD.begin()) 
  {
    Serial.println("SD card mount jednak zawiodl :("); // w przypadku niepowodzenia w trakcie komunikacji z kartą SD, wyświetl błąd w serial monitorze
    return;
  }

  else
  {
    Serial.println("SD card mount nie zawidol :)"); // w przypadku powodzenia, wyświetl informację o sukcesie
    return;
  }

  // pobierz rodzaj karty microSD

  uint8_t cardType = SD.cardType();


  if (cardType == CARD_NONE)
  {
    Serial.println("Brak karty SD"); // w przypadku barku karty w sockecie, wyświetl błąd w serial monitorze
    return;
  }

  else
  {
    Serial.println("Widze karte SD!"); // w przypadku powodzenia, wyświetl informację o sukcesie
    return;
  }


  // sprawdzanie, czy plik lista.txt jest zapisany na karcie

  if (SD.exists(fileName)) 
  {
    Serial.printf("Plik %s istnieje na karcie SD.\n", fileName);
  }

  else 
  {
    Serial.printf("Nie znalazlem pliku %s na karcie SD. Tworzenie nowego pliku...\n", fileName);

    // Tworzenie nowego pliku

    File file = SD.open(fileName);


    if (file) 
    {
      Serial.printf("Plik %s został utworzony pomyślnie.\n", fileName);
      file.close();
    } 

    else 
    {
      Serial.printf("Nie udalo sie utworzyc pliku %s.\n", fileName);
    }
  }


}


// całkowite działanie programu - funkcja główna.

void loop() 
{
  delay(100);
  
  // IDLE screen

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(getCurrentDate());
  lcd.setCursor(0, 1);
  lcd.print(getCurrentTime());
  

  // sprawdzamy stany przycisków 

  redButtonState = digitalRead(redButtonPin);
  yellowButtonState = digitalRead(yellowButtonPin);
  greenButtonState = digitalRead(greenButtonPin);


  // obsługa czerwonego przycisku do usuwania ostatniego rekordu z pliku txt 
  
  if (redButtonState == LOW) 
  {
    // sprawdzenie, czy plik istnieje

    if (SD.exists(fileName)) 
    {
      Serial.printf("Plik %s istnieje na karcie SD.\n", fileName);

      // otwieranie pliku w trybie odczytu

      File file = SD.open(fileName, FILE_READ);
      if (!file) 
      {
        Serial.println("Nie udało się otworzyć pliku do odczytu.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Blad odczytu.");
        delay(2000);
        return;
      }


      // podliczanie liczby linii w pliku

      int totalLines = 0;
      String fileContent = "";

      while (file.available()) 
      {
        String line = file.readStringUntil('\n');
        totalLines++;
        fileContent += line + '\n';
      }

      file.close();

      if (totalLines == 0) 
      {
        Serial.println("Plik jest pusty.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Plik pusty.");
        delay(2000);
        return;
      }


      // usuń ostatnią linię

      int lastLineStartIndex = fileContent.lastIndexOf('\n', fileContent.length() - 2); // szukanie przedostatniego '\n'
      if (totalLines == 1) 
      {
        fileContent = ""; // jeśli plik ma tylko jedną linię, usuń całą zawartość
      } 
      
      else if (lastLineStartIndex != -1) 
      {
        fileContent = fileContent.substring(0, lastLineStartIndex + 1); // przycinanie do przedostatniego '\n'
      }


      // Otwieramy plik w trybie zapisu

      file = SD.open(fileName, FILE_WRITE);
      if (file) 
      {
        file.print(fileContent); // zapisywanie zmodyfikowanej zawartości
        file.close();
        Serial.println("Usunięto ostatnią linię z pliku.");


        // wyświetlanie komunikatu na LCD

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Usunieto");
        lcd.setCursor(0, 1);
        lcd.print("ostatnia dawke.");
        delay(2000);
      } 
      
      else 
      {
        Serial.println("Nie udało się otworzyć pliku w trybie zapisu.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Blad zapisu.");
        delay(2000);
      }
    }

    else 
    {
      Serial.printf("Plik %s nie istnieje na karcie SD.\n", fileName);

      
      // wyświetlanie komunikatu na LCD

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Plik nie");
      lcd.setCursor(0, 1);
      lcd.print("istnieje.");
      delay(2000);
    }

  } 
  




  // obsługa żółtego przycisku do wyświetlenia ostatniego rekordu z pliku txt 

  else if (yellowButtonState == LOW) 
  {
    // sprawdzenie, czy plik istnieje

    if (!SD.exists(fileName)) 
    {
      Serial.printf("Plik %s nie istnieje na karcie SD.\n", fileName);  // w przypadku nie znalezienia pliku, wyświetl błąd w serial monitorze


      // wyświetlanie komunikatu o braku danych na LCD

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Brak danych.");
      delay(2000);
      return;
    }

    // otwieranie pliku w trybie odczytu

    File file = SD.open(fileName, FILE_READ);
    if (!file) 
    {
      Serial.println("Nie udało się otworzyć pliku do odczytu."); // w przypadku niepowodzenia w otwieraniu pliku, wyświetl błąd w serial monitorze
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Blad odczytu.");
      delay(2000);
      return;
    }


    // zmienna do przechowywania ostatniej linii

    String lastLine = "";
    String currentLine = "";


    // odczytywanie pliku linia po linii

    while (file.available()) 
    {
      char c = file.read(); // pobieranie znaku
      if (c == '\n') 
      {      
        lastLine = currentLine; // aktualna linia staje się ostatnią linią
        currentLine = "";       // resetowanie aktualnej linię
      }
      
      else 
      {
        currentLine += c; // dodanie znaku do aktualnej linii
      }
    }
    file.close();


    // sprawdzenie, czy udało się odczytać ostatnią linię

    if (lastLine == "") 
    {
      Serial.println("Plik jest pusty lub brak danych.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Brak danych.");
      delay(2000);
      return;
    }


    // rozdzielenie linii na datę i czas (w formacie "RRRR-MM-DD HH:MM")

    int spaceIndex = lastLine.indexOf(' '); // odszukanie pozycji spacji
    if (spaceIndex == -1) 
    {
      Serial.println("Nieprawidlowy format danych w pliku."); // w przypadku niepowodzenia z odczytem delimitera w pliku, wyświetl błąd w serial monitorze
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Blad formatu.");
      delay(2000);
      return;
    }

    String lastDate = lastLine.substring(0, spaceIndex);        // wyciąganie daty
    String lastTime = lastLine.substring(spaceIndex + 1);       // wyciąganie czasu


    // wyświetlenie daty i czasu na LCD

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ostatnia dawka:");
    lcd.setCursor(0, 1);
    lcd.print(lastDate);   
    delay(2000);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Godzina:");
    lcd.setCursor(0, 1);
    lcd.print(lastTime);   
    delay(2000);

    // zapisanie pobranej informacji z pliku txt w serial monitorze

    Serial.printf("Ostatnia dawka: %s %s\n", lastDate.c_str(), lastTime.c_str());
  }


  // obsługa zielonego przycisku do zapisania rekordu do pliku txt 

  else if (greenButtonState == LOW) 
  {
    // sprawdzanie, czy plik istnieje

    if (SD.exists(fileName)) 
    {
      Serial.printf("Plik %s istnieje na karcie SD.\n", fileName);
    } 

    else 
    {
      Serial.printf("Plik %s nie istnieje na karcie SD. Tworzenie nowego pliku...\n", fileName);
      File newFile = SD.open(fileName, FILE_WRITE);
      
      if (newFile) 
      {
        newFile.close(); // zamykanie nowo utworzony pusty plik
      }
      
      else 
      {
        Serial.println("Nie można utworzyć nowego pliku."); // w przypadku nie utworzenia pliku, wyświetl błąd w serial monitorze
        return;
      }
    }


    // pobranie aktualnej daty i godziny

    String saveDate = getCurrentDate();  
    String saveHour = getCurrentTime(); 
    String result = saveDate + " " + saveHour + "\n"; // łączenie daty i godziny z nową linią


    // otwieranie pliku w trybie dopisywania i zapisywanie danych

    File file = SD.open(fileName, FILE_APPEND);
    
    if (file) 
    {
      file.print(result); // dopisanie daty i godziny na końcu pliku
      file.close();       // zamykanie pliku po zakończeniu operacji

      Serial.printf("Zapisano do pliku: %s\n", result.c_str());


      // Wyświetlanie komunikatu na LCD

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Pomyslnie dodano");
      lcd.setCursor(0, 1);
      lcd.print("nowa dawke!");
      delay(2000);
    } 

    else 
    {
      Serial.println("Nie udało się otworzyć pliku w trybie dopisywania."); // w przypadku nie otwarcia pliku, wyświetl błąd w serial monitorze
    }
  }
}
