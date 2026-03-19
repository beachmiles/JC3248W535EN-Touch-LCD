#include <JC3248W535EN-Touch-LCD.h>
#include "ButtonGuiClass.h"     //uses the JC3248W535EN functions

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

static const String programVersion = "1.0.0";    //program version
uint16_t touchX, touchY;            //touch screen variables.

//vars for clock
static int16_t hh, mm, ss;
static unsigned long targetTime = 0; // next action time
unsigned long cur_millis;
String clockString;                  // holds the created time screen. Happens once a second

ButtonGuiClass LastTouch    (screenPtr, 325, 40, 150, 28, 255, 100, 100, "Last Touch", 2);   //button showing the last touch time
ButtonGuiClass CS           (screenPtr, 325, 70, 150, 28, 200, 255, 255, "Clear Screen", 2); //button to clear the screen
ButtonGuiClass ColorRand    (screenPtr, 325, 100, 150, 28, 100, 255, 255, "Random Color", 2);  //button to make a random background color
ButtonGuiClass ResetChip    (screenPtr, 325, 130, 150, 28, 0xFF, 0xFF, 0x00, "Reset Esp32", 2);   //button to reset esp32

void setup() {
  Serial.begin(115200);

  // Initialize the screen
  if (!screen.begin()) {
    Serial.println("Screen initialization failed!");
    return;
  }
  //gfx->setRotation(1);  //If using other gfx libaries prob need to do this. You have to use one or the other unfortuantly currently as setting this breaks what the guy did to get the Arduino_Canvas working on this board

  screen.clear(5, 70, 70); //set background to be dark teal
  LastTouch.displayButt();  //display the button
  CS.displayButt();         //display the button
  ColorRand.displayButt();  //display the button
  ResetChip.displayButt();  //display the button

  screen.setColor(255,255,255);                       //set text color
  screen.prt("Program Ver=" + programVersion,0,0,3);  //print text
  screen.setColor(255,100,100);      //set text color
  screen.prt("Font size-1",0,40,1);  //print text
  screen.prt("Font size-2",0,60,2);  //print text
  screen.prt("Font size-3",0,80,3);  //print text
  screen.prt("Font size-4",0,110,4); //print text
  screen.prt("Font size-5",0,140,5); //print text

  screen.setColor(10,90,100);                                    //set background color of rectangle
  screen.drawFillRect(0, 250, screen.width, screen.height - 250); //clear the status bar
  screen.setColor(50,255,255);               //set text color
  screen.prt("Click on buttons or screen to test functions. Testing the wrap around functon on screen. It should auto wrap",0,260,2);  //print text
  
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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
}

void loop() {
  processSerialCommand();
  handleTouchScreen();
  displayUptime();
}

//Note the touchscreen on this unit doesnt seem to be enabled around the outside edge of the display
void handleTouchScreen(){
  if (screen.getTouchPoint(touchX, touchY)){
    LastTouch.updateText("LT=" + clockString);

    //check if our last touch clock button was clicked
    if (LastTouch.checkIfClicked(touchX, touchY)){
      screen.setColor(50,50,50);          //set background color of rectangle
      screen.drawFillRect(0, 0, 320, 40); //clear the status bar
      screen.setColor(255,100,100);       //set text color
      screen.prt("Last touch button clicked",0,0,2); //print touch coords
      delay(50);
    }

    //check if our clear screen button was clicked
    else if (CS.checkIfClicked(touchX, touchY)){
      screen.clear(0,0,0);                //black out screen
      displayAllButtonsAndUpdatedText();

      screen.setColor(50,50,50);          //set background color of rectangle
      screen.drawFillRect(0, 0, 320, 40); //clear the status bar
      screen.setColor(255,255,255);       //set text color
      screen.prt("Screen Cleared",0,0,3); //print touch coords
      delay(50);
    }
    else if (ColorRand.checkIfClicked(touchX, touchY)){
      screen.clear(random(0, 255),random(0, 255),random(0, 255));                //set random color
      displayAllButtonsAndUpdatedText();

      screen.setColor(50,50,50);          //set background color of rectangle
      screen.drawFillRect(0, 0, 320, 40); //clear the status bar
      screen.setColor(255,100,100);       //set text color
      screen.prt("Random Background Color Picked",0,0,2);   //print touch coords
      delay(50);
    }
    else if (ResetChip.checkIfClicked(touchX, touchY)){
      screen.setColor(50,50,50);            //set background color of rectangle
      screen.drawFillRect(0, 0, 320, 40);   //clear the status bar
      screen.setColor(255,100,100);         //set text color
      screen.prt("Resetting esp32",0,0,3);  //print touch coords

      screen.flush();   //anytime you want the screen to update you have to do a flush
      delay(50);
      ESP.restart();    //restart esp32
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

    screen.flush();   //for all cases after a touch write to screen
    delay(10);
  }
}

void displayAllButtonsAndUpdatedText(){
  LastTouch.displayButt();            //display the button
  CS.displayButt();                   //redisplay button
  ColorRand.displayButt();            //redisplay button
  ResetChip.displayButt();            //redisplay button
  printCurrentClock();                //display clock
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