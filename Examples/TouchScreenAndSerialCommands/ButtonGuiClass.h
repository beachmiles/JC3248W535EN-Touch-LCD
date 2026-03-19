#include <JC3248W535EN-Touch-LCD.h>

//quick and dirty Button Class
class ButtonGuiClass {
  private:
    int PrivateVariable;

  public:
	  //Arduino_GFX *gfx;
    JC3248W535EN *screen;            //The main display variable

    int16_t X;
    int16_t Y;
    int16_t W;
    int16_t H;
    uint8_t Red_BG;
    uint8_t Green_BG;
    uint8_t Blue_BG;
    uint8_t fontSize;
    String Text;

    ButtonGuiClass(void){
      Text = "ClassNotInit";
    }

    ButtonGuiClass(JC3248W535EN *screenClass){
      screen = screenClass;
      Text = "TextBox";
    }

    ButtonGuiClass(JC3248W535EN *screenClass, int16_t XX, int16_t YY, int16_t WW, int16_t HH, uint8_t RedBG, uint8_t GreenBG, uint8_t BlueBG, String Textt, uint8_t fontSizee)
    {
      screen = screenClass;
      X = XX;
      Y = YY;
      W = WW;
      H = HH;

      //make sure we are within the bounds of the screen
      if (X > screen->width){ X = screen->width;}
      if (Y > screen->height){ Y = screen->height;}
      if (W > screen->width){ W = screen->width;}
      if (H > screen->height){ H = screen->height;}

      Red_BG   = RedBG;
      Green_BG = GreenBG;
      Blue_BG  = BlueBG;
      Text  = Textt;
      fontSize = fontSizee;
    }

    void displayButt(void){
      // Draw Button. Right now its a solid backgrund color. Could make it just an outlined color button or something fancier
      screen->setColor(Red_BG, Green_BG, Blue_BG);
      screen->drawFillRect(X, Y, W, H);
      //screen->setColor(255,255,255); //make white text the default for now
      screen->setColor(0,0,0); //make black text the default for now
      screen->prt(Text, X + 2, Y + 4, fontSize);
    }

    void AnimateButtPush(void){
      screen->setColor(255, 255, 255);  //simulate white flash
      screen->drawFillRect(X, Y, W, H);
      screen->flush();   //anytime you want the screen to update you have to do a flush
      //delay(5);

      // Draw Button 
      screen->setColor(Red_BG, Green_BG, Blue_BG);
      screen->drawFillRect(X, Y, W, H);
      //screen->setColor(255,255,255); //make white text the default for now
      screen->setColor(0,0,0); //make black text the default for now
      screen->prt(Text, X + 2, Y + 4, fontSize);
    }

    void updateText(String newText){
      Text = newText;
      displayButt();
    }

    //check if xy click is in our button
    bool checkIfClicked(uint16_t touchX, uint16_t touchY){
      if (touchX > screen->width){touchX = screen->width; }
      if (touchY > screen->height){touchY = screen->height; }

      if ( touchX >= X && touchX <= X+W && touchY >= Y && touchY <= Y+H){
        Serial.println("Button " + Text + " Clicked");
        AnimateButtPush();
        return true;
      }
      else{
        return false;
      }
    }
};  //end ButtonGuiClass