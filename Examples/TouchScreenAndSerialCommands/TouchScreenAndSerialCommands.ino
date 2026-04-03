#include <JC3248W535EN-Touch-LCD.h>
#include "ButtonGuiClass.h"     //uses the JC3248W535EN functions
//#include "glcdfont.h"

//Need to install these libraries 1st
//Verified with v1.6.5 GFX Library for Arduino / Arduino_GFX_Library.h 
//https://github.com/moononournation/Arduino_GFX
//https://github.com/Bodmer/JPEGDecoder

//And then install this JC3248W535EN-Touch-LCD library manually in Arduino by adding zip libarary.
//I started with this 0.9.6 version
//https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD/releases/tag/0.9.6
//then made updates here
//https://github.com/beachmiles/JC3248W535EN-Touch-LCD
//Have to use screen.flush() for anything to change on the display.

//Add SD librarys. Found in pincfg.j om DEMO_MJPEG folder
#include "SD_MMC.h"
#include "FS.h"
#include "pincfg.h"
//#include <vector>
/////////////// END SD CARD STUFF

//#define USE_HID_KEYBOARD
#ifdef USE_HID_KEYBOARD
  //Add keyboard libraries
  #include "USB.h"
  #include "USBHIDKeyboard.h"
  USBHIDKeyboard Keyboard;
#endif 

#include "soc/rtc_cntl_reg.h"       //has esp32 register for reboot to bootloader 

JC3248W535EN screen;                //The main display variable
JC3248W535EN *screenPtr = &screen;  //Pointer used for button functions
//Arduino_GFX *gfx = screen.gfx;      //in case anyone want to use direct gfx calls. Will not work seamlessly with "JC3248W535EN screen" as the rotation is different

static const String programVersion = "1.0.2";    //program version
uint16_t touchX, touchY;            //touch screen variables.
String curTestPanel = "Test";

//vars for clock
static int16_t hh, mm, ss;
static unsigned long targetTime = 0; // next action time
unsigned long cur_millis;
String clockString;                  // holds the created time screen. Happens once a second

//Test and Panel 1 buttons
ButtonGuiClass LastTouch    (screenPtr, 325, 40, 150, 28, 255, 100, 100, "Last Touch", 2);   //PANEL 1 button showing the last touch time
ButtonGuiClass CS           (screenPtr, 325, 70, 150, 28, 200, 255, 255, "Clear Screen", 2); //PANEL 1 button to clear the screen
ButtonGuiClass ColorRand    (screenPtr, 325, 100, 150, 28, 100, 255, 255, "Random Color", 2);  //PANEL 1 button to make a random background color
ButtonGuiClass ListSdFILES  (screenPtr, 325, 130, 150, 28, 255, 255, 255, "List Files", 2);  //PANEL 1 button to make a random background color
ButtonGuiClass TestModeBut  (screenPtr, 325, 160, 150, 28, 255, 255, 255, "TestMode Off", 2);  //PANEL 1 button to make a random background color

//Panel 2 buttons
ButtonGuiClass ResetChip    (screenPtr, 325, 40, 150, 28, 0xFF, 0xFF, 100, "Reset Esp32", 2);   //PANEL 2 button to reset esp32
ButtonGuiClass ResetToBL    (screenPtr, 325, 70, 150, 28, 0xFF, 100, 100, "Enter BootLDR", 2);   //PANEL 2 button to reset esp32

ButtonGuiClass Panel1Button (screenPtr, 0, 300, 230, 28, 200, 255, 255, "Switch to Panel 1", 2);     //button to switch to panel 1
ButtonGuiClass Panel2Button (screenPtr, 240, 300, 230, 28, 255, 255, 100, "Admin Settings", 2);   //button to switch to panel 2

void setup() {
  Serial.begin(115200);

  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    return;
  }
  //gfx->setRotation(1);  //If using other gfx libaries prob need to do this. You have to use one or the other unfortuantly currently as setting this breaks what the guy did to get the Arduino_Canvas working on this board
  //curTestPanel = "Panel1";
  curTestPanel = "Test";
  displayScreen();
  screen.flush(); //update display HAVE TO DO THIS ANYTIME YOU WANT THE SCREEN TO UPDATE! 

  // Print available serial commands
  Serial.println("Serial command interface ready!");
  Serial.println("Available commands (format: command|param1|param2|...):");
  Serial.println("  prt|text|x|y|size");
  Serial.println("  clear|r|g|b");
  Serial.println("  setColor|r|g|b");
  Serial.println("  drawQRCode|data|x|y|moduleSize|bgR|bgG|bgB|fgR|fgG|fgB");
  Serial.println("  drawFillRect|x|y|w|h");
  Serial.println("  drawRect|x|y|w|h");
  Serial.println("  drawLine|x0|y0|x1|y1");
  Serial.println("  drawFillCircle|x|y|radius");
  Serial.println("  drawCircleOutline|x|y|radius");
  Serial.println("  drawTriangle|x0|y0|x1|y1|x2|y2");
  Serial.println("  drawFillTriangle|x0|y0|x1|y1|x2|y2");
  Serial.println("  drawRoundRect|x|y|w|h|radius");
  Serial.println("  drawFillRoundRect|x|y|w|h|radius");
  Serial.println("  drawEllipse|x|y|rx|ry");
  Serial.println("  drawFillEllipse|x|y|rx|ry");
  Serial.println("  flush");

  //init SD card
  initSDcard();

  // initialize control over the keyboard:
  #ifdef USE_HID_KEYBOARD
    Keyboard.begin();
    USB.begin();
  #else
    Serial.println("\nUSE_HID_KEYBOARD not being used");
  #endif
}

void loop() {
  processSerialCommand();
  handleTouchScreen();
  displayUptime();
}

//Note the touchscreen on this unit doesnt seem to be enabled around the outside edge of the display
void handleTouchScreen(){

  // Check if the display was touched. touchX & touchY are GLOBAL VARS
  if (screen.getTouchPoint(touchX, touchY)){
    LastTouch.Text = "LT=" + clockString;         //update time on button but dont automatically re-display button
    //LastTouch.updateText("LT=" + clockString);  //update text and display button

    //check for clicks on Panel1
    if (curTestPanel == "Panel1" || curTestPanel == "Test" )
    {
      //check if our last touch clock button was clicked
      if (LastTouch.checkIfClicked(touchX, touchY)){
        screen.setColor(50,50,50);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //clear the status bar
        screen.setColor(255,100,100);       //set text color
        screen.prt("Last touch button clicked",0,0,2); //print touch coords
        delay(100);
      }

      //check if our clear screen button was clicked
      else if (CS.checkIfClicked(touchX, touchY)){
        screen.clear(0,0,0);                //black out screen
        //displayScreen();
        displayPanel1Buttons();

        screen.setColor(50,50,50);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //clear the status bar
        screen.setColor(255,255,255);       //set text color
        screen.prt("Screen Cleared",0,0,3); //print touch coords
        delay(100);
      }
      else if (ColorRand.checkIfClicked(touchX, touchY)){
        uint8_t randR = random(0, 255);
        uint8_t randG = random(0, 255);
        uint8_t randB = random(0, 255);
        screen.clear(randR,randG,randB);    //set random color
        displayScreen();

        screen.setColor(50,50,50);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //clear the status bar
        screen.setColor(255,100,100);       //set text color
        screen.prt("Random Background Color",0,0,2);   //print touch coords
        screen.prt("RGB=" + String(randR) + "," + String(randG) + "," + String(randB),0,20,2);   //print touch coords
        delay(100);
      }
      else if (ListSdFILES.checkIfClicked(touchX, touchY)){
        screen.clear(0,0,0);    //set random color
        displayScreen();

        screen.setColor(50,50,50);            //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
        screen.setColor(255,100,100);         //set text color
        screen.prt("Listing SD Files",0,0,2);  //print touch coords

        screen.setColor(255,255,255);          //set background color of rectangle
        Serial.println("Showing SD files");
        showSdFilesOnLCD();
        delay(100);
      }
      else if (TestModeBut.checkIfClicked(touchX, touchY)){
        //ButtonGuiClass TestModeBut  (screenPtr, 325, 160, 150, 28, 255, 255, 255, "TestMode Off", 2);  //PANEL 1 button to make a random background color
        if (TestModeBut.Text == "TestMode Off"){
          curTestPanel = "Test";
          TestModeBut.Text = "TestMode On";
        }
        else{
          curTestPanel = "Panel1";
          TestModeBut.Text = "TestMode Off";
        }

        screen.clear(0,0,0);    //set random color
        screen.setColor(50,50,50);            //set background color of rectangle

        displayScreen();
        delay(100);
      }
      //else test for other touches on touchscreen if in test mode and show keypresses
      else if (TestModeBut.Text == "TestMode On"){
        Serial.println("Touch Pressed:" + String(touchX) + "," + String(touchY));
        screen.setColor(0,50,100);          //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40); //we are 480 x 80

        screen.setColor(200,255,255);       //set text color
        screen.prt((String(touchX) + "," + String(touchY)),0,0,5);    //print touch coords

        screen.setColor(150,255,255);        //set color of circle
        screen.drawCircleOutline(touchX, touchY, 4);  //print tiny circle where touch happened
      }
    }
    else if (curTestPanel == "Panel2"){
      if (ResetChip.checkIfClicked(touchX, touchY)){
        screen.setColor(50,50,50);            //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
        screen.setColor(255,100,100);         //set text color
        screen.prt("Resetting esp32",0,0,3);  //print touch coords

        screen.flush();   //anytime you want the screen to update you have to do a flush
        getChipReadyForRestart();
        delay(50);
        ESP.restart();    //restart esp32
      }
      else if (ResetToBL.checkIfClicked(touchX, touchY)){
        screen.clear(0,0,0);                //black out screen
        screen.setColor(50,50,50);            //set background color of rectangle
        screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
        screen.setColor(255,100,100);         //set text color
        screen.prt("Entering Bootloader",0,0,2);  //print touch coords

        screen.setColor(255,255,255);         //set text color
        screen.prt("Esp32 will reboot into bootloader mode and provide an additonal serial port to use to flash device",0,40,2);  //print touch coords
        screen.flush();   //anytime you want the screen to update you have to do a flush
        getChipReadyForRestart(); //not really needed

        delay(50);
        REG_WRITE(RTC_CNTL_OPTION1_REG, 1); // 1 = Force Download Boot JUMP TO BOOTLOADER ON RESETART
        //esp_restart();  //old call to restart esp32
        ESP.restart();    //restart esp32
      }
    }


    checkPanelChangeButtons();  //do this for all cases as these buttons should be on every screen

    screen.flush();             //for all cases after a touch write to screen
    delay(20);                  //Try to prevent multiple touches with a delay
    screen.clearTouchData();    //Try to prevent multiple touches after a delay by clearing touch data
  } //end if
}

//screen flush happens at the end of handleTouchScreen function. touchX & touchY are GLOBAL VARS
void checkPanelChangeButtons(){
  if (Panel1Button.checkIfClicked(touchX, touchY)){
    curTestPanel = "Panel1";
    screen.clear(5, 50, 50); //clear background to a dark blue
    //screen.flush();   //for all cases after a touch write to screen
    displayScreen();
    delay(50);
  }
  else if (Panel2Button.checkIfClicked(touchX, touchY)){
    curTestPanel = "Panel2";
    screen.clear(0, 0, 0); //clear background to BLACK
    //screen.flush();   //for all cases after a touch write to screen
    displayScreen();
    delay(50);
  }
}


void displayScreen(){
  if (curTestPanel == "Panel1"){
    panel1Display();
  }
  else if (curTestPanel == "Panel2"){
    panel2Display();
  }
  //for unknown case show test screen
  else{
    //panel1Display();                    //show normal buttons
    displayTestScreen();
    screen.setColor(50,50,50);          //set background color of rectangle
    screen.drawFillRect(0, 0, 320, 40); //clear the status bar
    screen.setColor(255,100,100);       //set text color
    screen.prt("curTestPanel=" + curTestPanel,0,0,2);   //print touch coords
  }
}

void panel1Display(){
  screen.setColor(100,255,255);       //set text color
  screen.prt("Panel 1",0,0,3);   //print touch coords

  screen.setColor(255,255,255);       //set text color
  displayPanel1Buttons();
}

void displayPanel1Buttons(){
  LastTouch.displayButt();            //display the button
  CS.displayButt();                   //redisplay button
  ColorRand.displayButt();            //redisplay button
  ListSdFILES.displayButt();
  TestModeBut.displayButt();
  printCurrentClock();                //display clock1
  Panel1Button.displayButt();
  Panel2Button.displayButt();
}

void panel2Display(){
  screen.setColor(100,255,255);       //set text color
  screen.prt("Admin Settings",0,0,3);   //print touch coords

  screen.setColor(255,255,255);       //set text color white
  screen.prt("Admin Data 1=" + String(random(0,10)),0,40,2);   //print touch coords
  screen.prt("Admin Data 2=" + String(random(0,1000)),0,60,2);   //print touch coords
  screen.prt("Admin Data 3=" + String(random(0,1000)),0,80,2);   //print touch coords
  screen.prt("Admin Data 4=" + String(random(0,1000)),0,100,2);   //print touch coords
  screen.prt("Admin Data 5=" + String(random(0,1000)),0,120,2);   //print touch coords

  ResetChip.displayButt();            //redisplay button
  ResetToBL.displayButt();            //redisplay button
  printCurrentClock();                //display clock
  Panel1Button.displayButt();
  Panel2Button.displayButt();
}

void displayTestScreen(){
  screen.clear(5, 70, 70); //set background to be dark teal

  screen.setColor(255,255,255);                       //set text color
  //screen.setFont(&Roboto_Regular16pt7b);              //use roboto font. WARNING THIS IS A FIXED LARGE FONT
  //screen.setFont(NULL);                               //revert to original font?

  screen.prt("Program Ver=" + programVersion,0,0,3);  //print text
  screen.setColor(255,100,100);      //set text color
  screen.prt("Font size-1",0,40,1);  //print text
  screen.prt("Font size-2",0,60,2);  //print text
  screen.prt("Font size-3",0,80,3);  //print text
  screen.prt("Font size-4",0,110,4); //print text
  screen.prt("Font size-5",0,140,5); //print text

  screen.setColor(10,90,100);                                    //set background color of rectangle
  screen.drawFillRect(0, 210, screen.width, screen.height - 210); //clear the status bar
  screen.setColor(50,255,255);               //set text color
  screen.prt("Click on buttons or screen to test functions. Testing the wrap around functon on screen. It should auto wrap",0,220,2);  //print text
  
  TestModeBut.Text = "TestMode On";
  displayPanel1Buttons();

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //screen.flush(); //update display HAVE TO DO THIS ANYTIME YOU WANT THE SCREEN TO UPDATE! 
}

//should just use the button class for this and update the text
void printCurrentClock(){
  // clear time background
  screen.setColor(0,0,0);               //set background color of rectangle
  screen.drawFillRect(325, 2, 150, 35); //clear the bar

  //write the time
  screen.setColor(40,255,255);       //set background color of text
  screen.prt(clockString,335,12,2);   //print clock
}

void displayUptime(){
  cur_millis = millis();
  if (cur_millis >= targetTime)
  {
    targetTime += 1000;
    ss++; // Advance second
    if (ss == 60)
    {
      ss = 0;
      mm++; // Advance minute
      if (mm > 59)
      {
        mm = 0;
        hh++; // Advance hour
        if (hh > 23)
        {
          hh = 0;
        }
      }
    }

    clockString = "";
    if (hh < 10) {
      clockString = clockString + "0";
    }

    clockString = clockString + hh + ":";
    if (mm < 10) {
      clockString = clockString + "0";
    }

    clockString = clockString + mm + ":";
    if (ss < 10) {
      clockString = clockString + "0";
    }
    clockString = clockString + ss;
    
    printCurrentClock();
    screen.flush();
  }
}

void pressControlKey(char whichChar){
  #ifdef USE_HID_KEYBOARD
    //enter control alt delete
    Keyboard.press(KEY_LEFT_CTRL);
    //Keyboard.press(KEY_LEFT_ALT);
    //Keyboard.press(KEY_DELETE);
    delay(100);
    //Keyboard.press('c');
    Keyboard.press(whichChar);
    
    Keyboard.releaseAll();
    Keyboard.flush();
    delay(50);
  #else
    Serial.println("USE_HID_KEYBOARD not defined in .ino file");
  #endif
}

//shut down periphials. Mainly needed for bootloader mode maybe?
void getChipReadyForRestart(){
  SD_MMC.end(); //safley shut down SD card when done with it?
  //screen.end();
  //Keyboard.end();
}

void showSdFilesOnLCD(){
  listDir(SD_MMC, "/", 0, 1);
}

void initSDcard(){
  esp_rom_printf("Initialize tf card\n");
  //SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);

  if(! SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0)){
      Serial.println("SD_MMC.setPins change failed!");
      return;
  }
  else{
    Serial.println("SD_MMC.setPins success!");
  }

  //bool SDMMCFS::begin(const char *mountpoint, bool mode1bit, bool format_if_mount_failed, int sdmmc_frequency, uint8_t maxOpenFiles) 
  //if (!SD_MMC.begin()) 
  //if (!SD_MMC.begin("/sdcard", true)){
  //if (!SD_MMC.begin("/sdmmc", true, false, 20000))    //original from vendor
  if (!SD_MMC.begin("/sdcard", true, false, 20000))     //this works
  {
    esp_rom_printf("Card Mount Failed\n");
    return;
  }
  else{
    uint8_t cardType = SD_MMC.cardType();

    if (cardType == CARD_NONE) {
      Serial.println("No SD_MMC card attached");
      return;
    }

    Serial.print("SD_MMC Card Type: ");
    if (cardType == CARD_MMC) {
      Serial.println("MMC");
    } else if (cardType == CARD_SD) {
      Serial.println("SDSC");
    } else if (cardType == CARD_SDHC) {
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

    listDir(SD_MMC, "/", 0, 0);
    createDir(SD_MMC, "/mydir");
    listDir(SD_MMC, "/", 0, 0);
    removeDir(SD_MMC, "/mydir");
    listDir(SD_MMC, "/", 2, 0);
    writeFile(SD_MMC, "/hello.txt", "Hello ");
    appendFile(SD_MMC, "/hello.txt", "World!\n");
    readFile(SD_MMC, "/hello.txt");
    deleteFile(SD_MMC, "/foo.txt");
    renameFile(SD_MMC, "/hello.txt", "/foo.txt");
    readFile(SD_MMC, "/foo.txt");
    testFileIO(SD_MMC, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

    //SD_MMC.end(); //safley shut down SD card when done with it?
  }
}

void processSerialCommand() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    // Split the command by delimiter '|'
    int params[10];  // Array to store numeric parameters
    String textParam = "";  // For text parameter
    String parts[11];  // To store parts of the command
    
    int partCount = 0;
    int startIndex = 0;
    int delimiterIndex;
    
    while ((delimiterIndex = command.indexOf('|', startIndex)) != -1 && partCount < 11) {
      parts[partCount++] = command.substring(startIndex, delimiterIndex);
      startIndex = delimiterIndex + 1;
    }
    
    // Add the last part
    if (startIndex < command.length() && partCount < 11) {
      parts[partCount++] = command.substring(startIndex);
    }
    
    if (partCount < 1) {
      Serial.println("Invalid command format");
      return;
    }
    
    String cmd = parts[0];
    
    // Process based on command
    if (cmd == "prt" && partCount >= 4) {
      textParam = parts[1];
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+2].toInt();
      }
      screen.prt(textParam, params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Text printed: " + textParam);
    }
    else if (cmd == "clear" && partCount >= 1) {
      if (partCount >= 4) {
        for (int i = 0; i < 3; i++) {
          params[i] = parts[i+1].toInt();
        }
        screen.clear(params[0], params[1], params[2]);
      } else {
        screen.clear(); // Use default values
      }

      screen.flush();
      Serial.println("Screen cleared");
    }
    else if (cmd == "setColor" && partCount >= 4) {
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.setColor(params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Color set");
    }
    else if (cmd == "drawQRCode" && partCount >= 11) {
      textParam = parts[1];
      for (int i = 0; i < 9; i++) {
        params[i] = parts[i+2].toInt();
      }
      screen.drawQRCode(textParam.c_str(), params[0], params[1], params[2], 
                        params[3], params[4], params[5], params[6], params[7], params[8]);
      screen.flush();
      Serial.println("QR code drawn");
    }
    else if (cmd == "drawFillRect" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillRect(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Rectangle filled");
    }
    else if (cmd == "drawRect" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawRect(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Rectangle drawn");
    }
    else if (cmd == "drawLine" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawLine(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Line drawn");
    }
    else if (cmd == "drawFillCircle" && partCount >= 4) {
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillCircle(params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Circle filled");
    }
    else if (cmd == "drawCircleOutline" && partCount >= 4) {
      for (int i = 0; i < 3; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawCircleOutline(params[0], params[1], params[2]);
      screen.flush();
      Serial.println("Circle outline drawn");
    }
    else if (cmd == "drawTriangle" && partCount >= 7) {
      for (int i = 0; i < 6; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawTriangle(params[0], params[1], params[2], params[3], params[4], params[5]);
      screen.flush();
      Serial.println("Triangle drawn");
    }
    else if (cmd == "drawFillTriangle" && partCount >= 7) {
      for (int i = 0; i < 6; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillTriangle(params[0], params[1], params[2], params[3], params[4], params[5]);
      screen.flush();
      Serial.println("Triangle filled");
    }
    else if (cmd == "drawRoundRect" && partCount >= 6) {
      for (int i = 0; i < 5; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawRoundRect(params[0], params[1], params[2], params[3], params[4]);
      screen.flush();
      Serial.println("Rounded rectangle drawn");
    }
    else if (cmd == "drawFillRoundRect" && partCount >= 6) {
      for (int i = 0; i < 5; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillRoundRect(params[0], params[1], params[2], params[3], params[4]);
      screen.flush();
      Serial.println("Rounded rectangle filled");
    }
    else if (cmd == "drawEllipse" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawEllipse(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Ellipse drawn");
    }
    else if (cmd == "drawFillEllipse" && partCount >= 5) {
      for (int i = 0; i < 4; i++) {
        params[i] = parts[i+1].toInt();
      }
      screen.drawFillEllipse(params[0], params[1], params[2], params[3]);
      screen.flush();
      Serial.println("Ellipse filled");
    }
    else if (cmd == "flush") {
      screen.flush();
      Serial.println("Screen flushed");
    }
    else {
      Serial.println("Unknown or incomplete command: " + cmd);
    }
  }
}

/////////////////////////////// SD FUNCTIONS
void listDir(fs::FS &fs, const char *dirname, uint8_t levels, bool printOnLCD) {
  //screen.setColor(255,255,255);       //set text color white
  int currentLcdLine = 45;
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    if (printOnLCD){
      screen.prt("Failed to open directory",0,currentLcdLine,2);   //print touch coords
    }
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    if (printOnLCD){
      screen.prt("Not a directory",0,currentLcdLine,2); 
    }
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (printOnLCD){
        screen.prt("DIR:" + String(file.name()),0,currentLcdLine,1);
        currentLcdLine = currentLcdLine + 10;
      }

      if (levels) {
        listDir(fs, file.path(), levels - 1, printOnLCD);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());

      if (printOnLCD){
        screen.prt(String(file.name()) + " size=" + String(file.size()) ,5,currentLcdLine,1); 
        currentLcdLine = currentLcdLine + 10;
      }
    }
    file = root.openNextFile();
  }
}

void createDir(fs::FS &fs, const char *path) {
  Serial.printf("Creating Dir: %s\n", path);
  if (fs.mkdir(path)) {
    Serial.println("Dir created");
  } else {
    Serial.println("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path) {
  Serial.printf("Removing Dir: %s\n", path);
  if (fs.rmdir(path)) {
    Serial.println("Dir removed");
  } else {
    Serial.println("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while (file.available()) {
    Serial.write(file.read());
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("File renamed");
  } else {
    Serial.println("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path) {
  Serial.printf("Deleting file: %s\n", path);
  if (fs.remove(path)) {
    Serial.println("File deleted");
  } else {
    Serial.println("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char *path) {
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file) {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%zu bytes read for %lu ms\n", flen, end);
    file.close();
  } else {
    Serial.println("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++) {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes written for %lu ms\n", 2048 * 512, end);
  file.close();
}