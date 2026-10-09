# Medicine dose tracker
> An interactive electronics project developed as a university coursework assignment in collaboration with my partner, designed as a real-world simulation for practical use and learning.

## 1. Project Overview

This project is an ESP32-based dose tracking system designed to record and review the date and time of medication intake.

The device uses an LCD display to show the current date and time, which are synchronized over Wi-Fi using NTP servers. A microSD card is used to store a history of recorded doses in a text file.

The system is operated using three physical buttons, allowing users to add a new dose record, view the most recently recorded dose, or delete the last entry from the stored history.

The main goal of the project is to provide a simple, standalone device for tracking medication intake, with locally stored records that remain available on the microSD card.
<br><br>

<p align="center">
  <img src="assets/Fully assembled device.jpg"
  alt="Fully assembled device"
  width="400">
</p>

<br><br><br>

## 2. Features

* **Real-Time Clock Synchronization** — Synchronizes the system time over Wi-Fi using NTP servers.
* **LCD Display** — Displays the current date and time on a 16×2 character LCD.
* **Dose Recording** — Saves the date and time of each recorded dose to a text file on a microSD card.
* **Dose History** — Retrieves and displays the most recently recorded dose.
* **Record Deletion** — Allows the user to delete the last recorded entry.
* **Local Data Storage** — Stores records in a `lista.txt` file on the microSD card.
* **Physical Button Control** — Provides three dedicated buttons for recording, viewing, and deleting entries.
* **Serial Monitor Feedback** — Outputs status messages and error information for debugging and monitoring device operations.
* **Polish Time Zone Support** — Configures the system to use the Central European time zone, including daylight saving time.
<br><br><br>

## 3. Hardware Requirements


### 3.1. Circuit Diagram Without Programmer

<img src="assets/Circuit diagram without programmer.png" 
  alt="Circuit diagram without programmer" 
  width="700">
<br><br>

### 3.2. Circuit Diagram With Programmer

> **⚠️ Important - Power Supply Warning**
>
> When using the **Espressif Module Prog 1** programmer, make sure that the voltage regulator supplying the ESP32-DEVKIT-C is disconnected. The programmer directly supplies power to the development board, so powering the board simultaneously from the programmer and the external voltage regulator may cause damage to the ESP32-DEVKIT-C.
>
> The LCD and microSD card should remain powered as shown in the circuit diagram. Only the voltage regulator supplying the ESP32-DEVKIT-C must be disconnected when using the programmer.

<img src="assets/Circuit Diagram With Programmer.png"
  alt="Circuit diagram with programmer"
  width="900">
<br><br>

### 3.3. Assembled Circuit

<img src="assets/Assembled PCB.jpg"
  alt="Assembled PCB"
  width="900">
<br><br>

### 3.4. Tools and Consumables

<img src="assets/tools1.png"
  alt="Used tools #1"
  width="550">

<img src="assets/tools2.png"
  alt="Used tools #2"
  width="550">
<br><br>

### 3.5. Bill of Materials (BOM)

<img src="assets/Bom_part1.png"
  alt="Bill of materials #1"
  width="550" >

<img src="assets/bom_part2.png"
  alt="Bill of materials #2"
  width="550">
<br><br><br>


## 4. Software Requirements

### 4.1 Development Environment

- **Arduino IDE** — used to write, compile, and upload the firmware to the ESP32.
- **ESP32 Board Support Package** — required to compile and upload the program for the ESP32 platform.
<br><br>

### 4.2 Required Libraries

The project uses the following libraries:

| Library | Purpose |
|---|---|
| `WiFi.h` | Connects the ESP32 to a Wi-Fi network. |
| `time.h` | Provides date and time handling functions. |
| `esp_sntp.h` | Supports NTP time synchronization notifications. |
| `Wire.h` | Provides I2C communication. |
| `hd44780.h` | Provides functionality for the LCD display library. |
| `hd44780ioClass/hd44780_I2Cexp.h` | Supports LCD displays connected through an I2C expander. |
| `FS.h` | Provides filesystem interface functionality. |
| `SD.h` | Handles microSD card operations. |
| `SPI.h` | Provides SPI communication used by the SD card interface. |

The ESP32 Wi-Fi, time, filesystem, SD card, SPI, and I2C libraries are typically available through the ESP32 Arduino platform package. The `hd44780` library may need to be installed separately through the Arduino IDE Library Manager.
<br><br>

### 4.3. Configuration

Before uploading the firmware, configure the following parameters in the source code:

- **Wi-Fi credentials** — set `ssid` and `password` to match your network.
- **NTP servers** — configure the server addresses used to synchronize the system clock.
- **Time zone** — configure the time zone and daylight saving time rules for your location.
- **Serial baud rate** — the default value is `115200`.
- **Hardware pin assignments** — verify that the GPIO assignments match your wiring.

Make sure that the ESP32 is connected to a working Wi-Fi network and that the microSD card and LCD are wired correctly before running the device. Also, keep in mind that the microSD card should be formatted as **FAT32**; otherwise, the ESP32 may fail to recognize or initialize the card.
<br><br><br>

## 5. How It Works

### 5.1. Device Initialization

When the ESP32 starts, the firmware initializes serial communication, connects to the configured Wi-Fi network, configures NTP time synchronization, initializes the LCD display, and sets up the three buttons using internal pull-up resistors.

The program also attempts to initialize the microSD card.
<br><br>

### 5.2. Displaying the Current Date and Time

During normal operation, the LCD displays the current date and time obtained from the ESP32 system clock.

The device uses NTP servers to synchronize its clock over Wi-Fi and is configured for the Polish time zone, including daylight saving time.
<br><br>

### 5.3. Button Controls

The device is operated using three physical buttons connected to the ESP32.

| Button | Function |
|---|---|
| Red | Deletes the last recorded entry from the text file. |
| Yellow | Displays the date and time of the most recently recorded entry. |
| Green | Records a new dose by saving the current date and time. |

The buttons use `INPUT_PULLUP`, meaning that a button press is detected when the corresponding GPIO input reads `LOW`.
<br><br>

#### 5.3.1 Recording a Dose

When the green button is pressed, the firmware checks whether the history file exists on the microSD card. If necessary, it attempts to create the file.

The current date and time are then combined into a single record and appended to `lista.txt`.

Each record is intended to contain a date and time separated by a space, followed by a newline character.
<br><br>

#### 5.3.2 Viewing the Last Recorded Dose

When the yellow button is pressed, the firmware opens the history file and reads its contents to identify the most recent record.

The date and time are extracted from the record and displayed on the LCD in two separate screens.

If the file is missing, empty, or contains data in an unexpected format, the device displays an appropriate error or information message.
<br><br>

#### 5.3.3. Deleting the Last Recorded Dose

When the red button is pressed, the firmware reads the history file, identifies the last entry, and attempts to remove it from the stored contents.

The modified contents are then written back to the file. A confirmation or error message is displayed on the LCD.
<br><br>

### 5.4. Status Messages and Error Handling

The firmware uses the Serial Monitor to report operations such as Wi-Fi connection attempts, file access, data recording, and errors.

The LCD also displays short messages to inform the user about successful operations or problems encountered during device operation.
<br><br><br>

## 6. Data Storage

Dose records are stored on a microSD card in a text file named `lista.txt`. Each entry contains the date and time of the recorded dose. The file can be accessed on a computer using a card reader.

### 6.1. Example File Structure

The image below shows an example of the data stored in `lista.txt`.

<p align="center">
  <img src="assets/Lista structure.png" 
  alt="Example of the lista.txt file structure" 
  width="450">
</p>
