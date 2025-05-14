//OSU UWRT COM SCALE CODE
//Author: Nathan Becker
//Last modified date: 4/12/22

//Imported Libraries
#include <LiquidCrystal.h>
#include <HX711.h>

//Load Cell Pins
uint8_t loadA_data = 8;
uint8_t loadA_clock = 9;
uint8_t loadB_data = 10;
uint8_t loadB_clock = 11;
uint8_t loadC_data = 12;
uint8_t loadC_clock = 13;

//LCD Pins
const int rs = 2;
const int en = 3;
const int d4 = 4;
const int d5 = 5;
const int d6 = 6;
const int d7 = 7;

//Joystick Pins
const int button = A0;
const int VRy = A5;

//Set up variables
const float x_offset = -13.95452244;
const float y_offset = -14.64664725;
const float z_offset = -11.0550463;
const float len = 8.60774699; //side length of weighing triangle
const float theta = 20      //Angle of platform in degrees
                    * 3.1415926535897932384626433832795 / 180;
const float d = 5.60432993-0.0881996;
double X1 = 0, Y1 = 0, X2 = 0, Y2 = 0;
double Xr, Yr, Zr;
double x, y, x_sum = 0, xr_sum = 0, y_sum = 0, yr_sum = 0, z_sum = 0, zr_sum = 0;
String str;
int r_counter = 0, counter = 0;
int menu = 0, line = 1, menu_line = 0;   //LCD menu variables
bool cursor_flag = false, button_flag = false, joy_flag = false;
unsigned long timer = millis(), k = 0;

//Menu text
char *menus[][16] = {
  //Home menu
  { "-----Home------",
    "Measure        ",
    "Raw measure    ",
    "See prev result",
    "See avg result "
  },
  //Measure results screen
  { "----Results----",
    "Xr:            ",
    "Yr:            ",
    "Zr:            ",
    "Add to average ",
    "Return home    "
  },
  //Raw measure menu
  { "---Raw Measure--",
    "LA:             ",
    "LB:             ",
    "LC:             ",
    "X:              ",
    "Y:              ",
    "W:              ",
    "Tare            ",
    "Return home     "
  },
  //Previous Results
  { "-Prev Results--",
    "Xr:             ",
    "Yr:             ",
    "Zr:             ",
    "X1:             ",
    "Y1:             ",
    "X2:             ",
    "Y2:             ",
    "Return home     "
  },
  //Average Results
  { "--Avg Result---",
    "Xr:             ",
    "Yr:             ",
    "Zr:             ",
    "#:              ",
    "Clear average   ",
    "Return home     "
  }
};

//Create load cell objects
HX711 loadA;
HX711 loadB;
HX711 loadC;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);



void setup()
{
  //Begin communications with computer
  Serial.begin(9600);

  //Run set up functions
  lcd.begin(16, 2);
  loadA.begin(loadA_data, loadA_clock);
  loadB.begin(loadB_data, loadB_clock);
  loadC.begin(loadC_data, loadC_clock);

  //Set calibration for load cells
  loadA.set_scale(45907.12);  //45942.85  // <-- ENTER CALIBRATION NUMBERS IN PARENTHESIS
  loadB.set_scale(47591.28);  //47566.16
  loadC.set_scale(48180.00);  //48154.15

  //Joystick pin modes
  pinMode(VRy, INPUT);
  pinMode(button, INPUT_PULLUP);
  update_lcd();
}



void loop()
{
  //Respond to joystick input
  joystick();

  //Respond to button input
  if (check_button())
  {
    button_pressed();
    update_lcd();
  }
  //Used to blink cursor and update values every 700ms
  if (millis() % 700 == 0)
  {
    //Update Raw measure screen
    if (menu == 2)
    {
      float a, b, c;
      //only updates readings every 3 seconds
      if (k - millis() > 3000)
      {
        //read new load cell values
        k = millis();
        a = loadA.get_units(1);
        b = loadB.get_units(1);
        c = loadC.get_units(1);

        //Print new readings to the terminal
        Serial.println(a);
        Serial.println(b);
        Serial.println(c);
      }

      //Update raw measure menu text
      str = "LA: " + String(a,3).substring(0, 12);
      str.toCharArray(menus[menu][1], 17);
      str = "LB: " + String(b,3).substring(0, 12);
      str.toCharArray(menus[menu][2], 17);
      str = "LC: " + String(c, 3).substring(0, 12);
      str.toCharArray(menus[menu][3], 17);
      str = "X: " + String(calc_x(a, b, c),5).substring(0, 12);
      str.toCharArray(menus[menu][4], 17);
      str = "Y: " + String(calc_y(a, b, c),5).substring(0, 12);
      str.toCharArray(menus[menu][5], 17);
      str = "W(kg): " + String((a + b + c)*0.453592,3).substring(0, 12);
      str.toCharArray(menus[menu][6], 17);
    }
    update_lcd();
  }
}



void joystick()
{
  //check if joystick is moved
  if (analogRead(VRy) < 300 && joy_flag == false)
  {
    joy_flag = true;
    //joystick moved up
    if (line == 0 && menu_line > 0)
    {
      //move menu up a line
      menu_line--;
      if (menu_line == 0)
      {
        line = 1;
      }
    }
    else if (menu_line != 0)
    {
      //move cursor up
      line = 0;
    }
    //update the lcd screen with new cursor and menu lines
    update_lcd();
  }
  else if (analogRead(VRy) > 700 && joy_flag == false)
  {
    joy_flag = true;
    if (line == 1 && menu_line + 1 < array_size(menu) - 1)
    {
      //move menu down a line
      menu_line++;
    }
    else
    {
      //move cursor down a line
      line = 1;
    }
    //update the lcd screen with new cursor and menu lines
    update_lcd();
  }
  else if (analogRead(VRy) <= 700 && analogRead(VRy) >= 300 && joy_flag == true)
  {
    //joystick moved back to center, new input can now be detected
    joy_flag = false;
  }
}

//returns the size of an array
int array_size(int num)
{
  int count = 0;
  for (int i = 0; i < 16; i++)
  {
    if (menus[num][i] != NULL)
    {
      count++;
    }
  }
  return count;
}

bool check_button()
{
  //if button is pressed down...
  if (digitalRead(button) == LOW)
  {
    //reset timer
    timer = millis();
    if (button_flag == false)
    {
      //
      button_flag = true;
      return true;
    }
  }
  //if button has been unpressed for atleast 100ms...
  else if (digitalRead(button) == HIGH && millis() - timer > 100)
  {
    //Button can now recieve inputs
    button_flag = false;
  }
  return false;
}

void button_pressed()
{
  switch (menu)
  {
    //User is on the home screen
    case 0:
      switch (menu_line + line)
      {
        case 4:
          menu = 4;
          menu_line = 0;
          line = 1;
          //Update avg menu with avg values
          str = "Xr: " + String(xr_sum / r_counter).substring(0, 12);
          str.toCharArray(menus[4][1], 16);
          str = "Yr: " + String(yr_sum / r_counter).substring(0, 12);
          str.toCharArray(menus[4][2], 16);
          str = "Zr: " + String(zr_sum / r_counter).substring(0, 12);
          str.toCharArray(menus[4][3], 16);
          str = "#: " + String(r_counter).substring(0, 12);
          str.toCharArray(menus[4][4], 16);
          break;
        //User wants to measure
        case 1:
          measure();
          break;
        //User wants to raw measure
        case 2:
          menu = 2;
          menu_line = 0;
          line = 1;
          break;
        //User wants to see prev results
        case 3:
          menu = 3;
          menu_line = 0;
          line = 1;
          //Update prev menu with prev results
          str = "Xr: " + String(Xr).substring(0, 12);
          str.toCharArray(menus[3][1], 16);
          str = "Yr: " + String(Yr).substring(0, 12);
          str.toCharArray(menus[3][2], 16);
          str = "Zr: " + String(Zr).substring(0, 12);
          str.toCharArray(menus[3][3], 16);
          str = "X1: " + String(X1).substring(0, 12);
          str.toCharArray(menus[3][4], 16);
          str = "Y1: " + String(Y1).substring(0, 12);
          str.toCharArray(menus[3][5], 16);
          str = "X2: " + String(X2).substring(0, 12);
          str.toCharArray(menus[3][6], 16);
          str = "Y2: " + String(Y2).substring(0, 12);
          str.toCharArray(menus[3][7], 16);
          break;

      }
      break;
    //User is on the measure results screen
    case 1:
      //return home
      if (menu_line + line == 5)
      {
        menu = 0;
        menu_line = 0;
        line = 1;
      }
      //Add to average
      if (menu_line + line == 4)
      {
        xr_sum += Xr;
        yr_sum += Yr;
        zr_sum += Zr;
        r_counter++;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Result added to");
        lcd.setCursor(0, 1);
        lcd.print("average...");
        delay(2500);
        //return home
        menu = 0;
        menu_line = 0;
        line = 1;
      }
      break;
    //User is on Raw Measure screen
    case 2:
      //tare scale
      if (menu_line + line == 7)
      {
        tare_scale();
      }
      //return home
      else if (menu_line + line == 8)
      {
        menu = 0;
        menu_line = 0;
        line = 1;
      }
      break;
    //User is on prev results
    case 3:
      if (menu_line + line == 8)
      {
        menu = 0;
        menu_line = 0;
        line = 1;
      }
      break;
    //User is on see avg results screen
    case 4:
      if (menu_line + line == 6)
      {
        menu = 0;
        menu_line = 0;
        line = 1;
      }
      else if (menu_line + line == 5)
      {
        xr_sum = 0;
        yr_sum = 0;
        zr_sum = 0;
        r_counter = 0;
        str = "Xr: " + String(xr_sum / r_counter).substring(0, 12);
        str.toCharArray(menus[4][1], 16);
        str = "Yr: " + String(yr_sum / r_counter).substring(0, 12);
        str.toCharArray(menus[4][2], 16);
        str = "Zr: " + String(zr_sum / r_counter).substring(0, 12);
        str.toCharArray(menus[4][3], 16);
        str = "#: " + String(r_counter).substring(0, 12);
        str.toCharArray(menus[4][4], 16);
        update_lcd();
      }
      break;
  }
}

void update_lcd()
{
  //clear screen
  lcd.clear();

  //Draw cursor on LCD screen
  lcd.setCursor(0, line);

  //Draws blinking cursor >
  if (cursor_flag)
  {
    cursor_flag = !cursor_flag;
    lcd.print(">");
  }
  else
  {
    cursor_flag = !cursor_flag;
    lcd.print(" ");
  }

  //write text on screen
  if (menu_line == 0)
  {
    lcd.setCursor(0, 0);
    lcd.print(menus[menu][menu_line]);
  }
  else
  {
    lcd.setCursor(1, 0);
    lcd.print(menus[menu][menu_line]);
  }
  lcd.setCursor(1, 0);
  lcd.print(menus[menu][menu_line]);
  lcd.setCursor(1, 1);
  lcd.print(menus[menu][menu_line + 1]);
}

void measure()
{
  //Informing user to remove robot
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Remove robot and");
  lcd.setCursor(0, 1);
  lcd.print("press button");

  //waiting for button press
  while (!check_button()) {}
  //Tare the scale once there is no robot on it
  tare_scale();

  //Informing user to place robot
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place robot then");
  lcd.setCursor(0, 1);
  lcd.print("press button");
  //Waiting for user to place robot and press button
  while (!check_button()) {}

  //Start measuring results
  counter = 0;
  x_sum = 0;
  y_sum = 0;
  int refresh = millis() - 201;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Measuring...");

  //measure for 10 seconds and get average
  float a, b, c;
  while (millis() - timer < 10000)
  {
    //read load cells
    a = loadA.get_units(1);
    b = loadB.get_units(1);
    c = loadC.get_units(1);
    //calculate 2D x, y point of COM and update average counters
    x = calc_x(a, b, c);
    y = calc_y(a, b, c);
    x_sum += x;
    y_sum += y;
    counter++;

    //update the screen every 200 ms with results
    if ((millis() - refresh) > 200)
    {
      refresh = millis();
      lcd.setCursor(0, 1);
      lcd.print(" X=" + String(x).substring(0, 5));
      lcd.setCursor(8, 1);
      lcd.print(" Y=" + String(y).substring(0, 5));
    }
    delay(1);
  }

  //Store measurement averages as X1 and Y1 points
  X1 = x_sum / counter;
  Y1 = y_sum / counter;
  Serial.println(X1);
  Serial.println(Y1);
  
  //first measurement done, tell use to flip robot
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Flip robot then");
  lcd.setCursor(0, 1);
  lcd.print("press button");

  //Wait for button press
  while (!check_button()) {}

  //Start measuring results
  counter = 0;
  x_sum = 0;
  y_sum = 0;
  refresh = millis() - 201;
  Serial.println("CLEAR!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Measuring...");

  //measure for 10 seconds and get average
  while (millis() - timer < 10000)
  {
    //read load cells
    a = loadA.get_units(1);
    b = loadB.get_units(1);
    c = loadC.get_units(1);
    //calculate 2D x, y point of COM and update average counters
    x = calc_x(a, b, c);
    y = calc_y(a, b, c);
    x_sum += x;
    y_sum += y;
    counter++;

    //update the screen every 200 ms with results
    if (millis() - refresh > 200)
    {
      refresh = millis();
      lcd.setCursor(0, 1);
      lcd.print(" X=" + String(x,3).substring(0, 5));
      lcd.setCursor(8, 1);
      lcd.print(" Y=" + String(y,3).substring(0, 5));
    }
    delay(5);
  }

  //Store measurement averages as X2 and Y2 points
  X2 = x_sum / counter;
  Y2 = y_sum / counter;
  Serial.println("X2: " + String(X2,5));
  Serial.println("Y2: " + String(Y2, 5));
  
  //measurements taken now calc and show results
  //adding offset and converting to meters
  Xr = (calc_Xr() + x_offset)*0.0254;
  Yr = calc_Yr();
  Zr = (calc_Zr() + z_offset)*0.0254;
  Yr = (Yr + y_offset)*-0.0254;
  Serial.println("Xr: " + String(Xr, 5));
  Serial.println("Yr: " + String(Yr, 5));
  Serial.println("Zr: " + String(Zr,5));

  
  menu = 1;
  menu_line = 0;
  line = 1;
  //Rewriting values for Xr, Yr and Zr for menu
  str = "Xr: " + String(Xr, 3).substring(0, 12);
  str.toCharArray(menus[menu][1], 17);
  str = "Yr: " + String(Yr, 3).substring(0, 12);
  str.toCharArray(menus[menu][2], 17);
  str = "Zr: " + String(Zr, 3).substring(0, 12);
  str.toCharArray(menus[menu][3], 17);
  update_lcd();
}


//Calculates 2D x cordinate of COM given load cell readings
float calc_x(float a, float b, float c)
{
  //Pluggin load cell values into equation. Math for equations documented in COM folder
  float x = len * (b / (2 * (a + b + c)) + c / (a + b + c));
  return x;
}

//Calculates 2D y cordinate of COM given load cell readings
float calc_y(float a, float b, float c)
{
  //Pluggin load cell values into equation. Math for equations documented in COM folder
  float y = sqrt(3.0) / 2 * (b / (a + b + c)) * len;
  return y;
}

//Calculates 3D x cordinate of the COM relative to the robot
float calc_Xr()
{
  //Using X1, Y1, and X2, Y2 to find COM, math documented in COM folder
  return (X2 - X1) / 2;
}

//Calculates 3D y cordinate of the COM relative to the robot
float calc_Yr()
{
  //Using X1, Y1, and X2, Y2 to find COM, math documented in COM folder
  return (Y1 - Y2) / ((cos(theta) * d - Y1) / (sec(theta) * Y1 - d) + (cos(theta) * d - Y2) / (sec(theta) * Y2 - d));
}

//Calculates 3D z cordinate of the COM relative to the robot
//MUST CALCULATE YR FIRST!!!!
float calc_Zr()
{
  //Using X1, Y1, and X2, Y2 to find COM, math documented in COM folder
  return ((cot(theta) * d - csc(theta) * Y1) / (sec(theta) * Y1 - d)) * Yr + cot(theta) * d - csc(theta) * Y1;
}

//Tares load cells to set their readings to zero
void tare_scale()
{
  loadA.tare();
  loadB.tare();
  loadC.tare();
}

//trig functions
float sec(float angle)
{
  return 1 / cos(angle);
}
float csc(float angle)
{
  return 1 / sin(angle);
}
float cot(float angle)
{
  return 1 / tan(angle);
}
