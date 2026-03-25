#include <JC3248W535EN-Touch-LCD.h>
#include "ButtonGuiClass.h"     //uses the JC3248W535EN functions
//#include "glcdfont.h"

//Need to install these libraries 1st
//Verified with v1.6.5 GFX Library for Arduino / Arduino_GFX_Library.h 
//https://github.com/moononournation/Arduino_GFX
//https://github.com/Bodmer/JPEGDecoder

//And then install this JC3248W535EN-Touch-LCD library manually in Arduino by adding zip libarary.
//I tested with the 0.9.6 version
//https://github.com/AudunKodehode/JC3248W535EN-Touch-LCD/releases/tag/0.9.6

//Have to use screen.flush() for anything to change on the display.

JC3248W535EN screen;                //The main display variable
JC3248W535EN *screenPtr = &screen;  //Pointer used for button functions
//Arduino_GFX *gfx = screen.gfx;      //in case anyone want to use direct gfx calls. Will not work seamlessly with "JC3248W535EN screen" as the rotation is different

static const String programVersion = "1.0.1";    //program version
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

//Panel 2 buttons
ButtonGuiClass ResetChip    (screenPtr, 325, 40, 150, 28, 0xFF, 0xFF, 100, "Reset Esp32", 2);   //PANEL 2 button to reset esp32

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
  curTestPanel = "Test";
  displayScreen();
  curTestPanel = "Panel1";

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

    //for now consider the Test and Panel1 to have the same button layout
    if (curTestPanel == "Test" || curTestPanel == "Panel1")
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
      //else test for other touches on touchscreen
      else{
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
        delay(50);
        ESP.restart();    //restart esp32
      }
    }

    checkPanelChangeButtons();  //do this for all cases as these buttons should be on every screen

    screen.flush();             //for all cases after a touch write to screen
    delay(20);                  //Try to prevent multiple touches with a delay
    screen.clearTouchData();    //Try to prevent multiple touches after a delay by clearing touch data
  }
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
  if (curTestPanel == "Test"){
    displayTestScreen();
  }
  else if (curTestPanel == "Panel1"){
    panel1Display();
  }
  else if (curTestPanel == "Panel2"){
    panel2Display();
  }
  //for unknown case show test screen
  else{
    panel1Display();
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
  screen.prt("Random Data 1..........",0,40,2);   //print touch coords
  screen.prt("Random Data 2..........",0,60,2);   //print touch coords
  screen.prt("Random Data 3..........",0,80,2);   //print touch coords
  displayPanel1Buttons();
}

void displayPanel1Buttons(){
  LastTouch.displayButt();            //display the button
  CS.displayButt();                   //redisplay button
  ColorRand.displayButt();            //redisplay button
  printCurrentClock();                //display clock
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
  
  LastTouch.displayButt();            //display the button
  CS.displayButt();                   //redisplay button
  ColorRand.displayButt();            //redisplay button
  printCurrentClock();                //display clock
  Panel1Button.displayButt();
  Panel2Button.displayButt();

  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  screen.flush(); //update display HAVE TO DO THIS ANYTIME YOU WANT THE SCREEN TO UPDATE! 
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