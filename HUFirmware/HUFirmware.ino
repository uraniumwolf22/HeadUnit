

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define BOOT_DELAY 1000
#define SKIP_BOOT_SEQ false
#define SCREEN_WIDTH 128     //I think you can derive this one..
#define SCREEN_HEIGHT 64

#define MAX_DATA_LENGTH 200 //set maximum data length for serial
#define NUM_STATS 8

#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D
#define SCREEN_ADDRESS_2 0x3C

#define ENC_A 33  //encoder pin A
#define ENC_B 32
#define ENC_SW

#define DEBUG_MODE true


enum DisplayMode {
  //menu modes vv
  MODEMENU,
  STATMENU,
  VOLUME,
  //status modes vv
  BIGSTATUS,
  SMALLSTATUS,
  GRAPH
};

struct Display {        //struct to store display proprties
  Adafruit_SSD1306* ctx;        //I2C Display object
  DisplayMode activeMode;      //display mode that is currently being rendered
  DisplayMode displayMode;     //last status screen that was being rendered

  int cursor_index;
  //Active statistics masks
  bool bigstatus_mask[NUM_STATS];
  bool smallstatus_mask[NUM_STATS];
  bool graph_mask[NUM_STATS];
};

struct Statistic {    //stores individual statistic properties
  char pid;
  char* name;
  char* unit;
  int min;          //minimum value for statistic
  int max;
  int danger;       //threshold for triggering warning
  int value;
};

const struct Statistic stats[NUM_STATS] = {   //list and default values of current supported statistics
  {0x5C, "OIL", "F", 0, 100, 80, 0},
  {0x05, "COOL", "F", 0, 100, 80, 0},
  {0x0E, "ADV", "deg", 0, 100, 80, 0},
  {0x24, "AFR", "", 0, 100, 80, 0},
  {0x06, "STFT", "%", 0, 100, 80, 0},
  {0x07, "LTFT", "%", 0, 100, 80, 0},
  {0x0F, "INTK", "F", 0, 100, 80, 0},
  {0x05, "RPM", "", 0, 100, 80, 0},
};



void drawStatMenu(Display& display) {
    const bool flipped = (display.ctx->getRotation() == 2);
    const int offset = flipped ? 0 : 16;

    // Choose selection mask based on current display mode
    bool selection[NUM_STATS];
    switch (display.displayMode) {
    case BIGSTATUS:
        memcpy(selection, display.bigstatus_mask, sizeof(int) * NUM_STATS); break;
    case SMALLSTATUS:
        memcpy(selection, display.smallstatus_mask, sizeof(int) * NUM_STATS); break;
    case GRAPH:
        memcpy(selection, display.graph_mask, sizeof(int) * NUM_STATS); break;
    }

    display.ctx->setTextSize(1); // 7px tall (8 is ideal)

    // Draw statistic buttons
    for (int i = 0; i < min(NUM_STATS, 16); i++) {
        const int x = (i % 4) * 32;
        const int y = (i / 4) * 12 + offset;

        int text_width;
        display.ctx->getTextBounds(stats[i].name, 0, 0,NULL, NULL, &text_width, NULL);
        display.ctx->setCursor(x + (32 - text_width) / 2, y + 2);  // Center text in bounding box

        if (selection[i]) {
            // Draw highlighted text
            display.ctx->fillRect(x, y, 32, 12, SSD1306_WHITE);
            display.ctx->setTextColor(SSD1306_BLACK);
            display.ctx->println(stats[i].name);

            // Draw cursor if selected
            if (display.cursor_index == i) {
                display.ctx->drawFastHLine(x + 4, y + 10, 24, SSD1306_BLACK);
            }
        }
        else {
            // Draw unhighlighted text
            display.ctx->setTextColor(SSD1306_WHITE);
            display.ctx->println(stats[i].name);

            // Draw cursor if selected
            if (display.cursor_index == i) {
                display.ctx->drawFastHLine(x + 4, y + 10, 24, SSD1306_WHITE);
            }
        }
    }

  // Draw 'Back' button
  // Width should be hardcoded once we choose a font
  int text_width;
  display.ctx->getTextBounds("BACK", 0, 0, NULL, NULL, &text_width, NULL);
  const int x = 64 - (text_width / 2);
  const int y = flipped ? 52 : 4;

  display.ctx->setCursor(x, y);
  display.ctx->println("BACK");

  if (display.cursor_index == -1) {
    display.ctx->drawFastHLine(x, y+9, text_width, SSD1306_WHITE);

  }
}



void drawDisplay(Display &display) {   //draw current mode onto correct display
  switch (display.activeMode) {
    // case MODEMENU: drawModeMenu(display); break;
    case STATMENU: drawStatMenu(display); break;
    // case VOLUME: drawVolume(display); break;
    // case BIGSTATUS: drawBigStatus(display); break;
    // case SMALLSTATUS: drawSmallStatus(display); break;
    // case GRAPH: drawGraph(display); break;
  }
}

struct Display top_display;
struct Display low_display;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //OLED Display object
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {

  Serial.begin(115200);

  display.setRotation(2);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { //initialize display
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

    if (!display2.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS_2)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  top_display.ctx = &display;
  low_display.ctx = &display2;


  //JUST FOR TESTING

  //Initial values
  top_display.displayMode = SMALLSTATUS;
  top_display.activeMode = STATMENU;
  top_display.cursor_index = 1;
  bool test_mask[NUM_STATS] = {0,0,0,0,0,0,0,0};   //set Initial Status mask
  memcpy(top_display.smallstatus_mask, test_mask, sizeof(test_mask));

  display.clearDisplay();
  display2.clearDisplay();
}

//Yeah Yeah there are tons of comments becuse this one hurts my small brain
char incoming_data[MAX_DATA_LENGTH] = "";  //Place to store incoming serial data
bool SERIAL_RECIEVED = false;             //Place to store current state of serial input buffer

void serialEvent() {                      //runs asyncronously with main loop
  static int data_index = 0;                 // Index for storing incoming data

  while (Serial.available()) {                  //check if serial data is in buffer
    char serialBuff = (char)Serial.read();      //set input char to serial buffer

    if (serialBuff == '\n') {                   //check if serial communication frame has been terminated
      incoming_data[data_index] = '\0';             // Terminate the string
      SERIAL_RECIEVED = true;
      data_index = 0;                              //Reset data index
    } else if (data_index < MAX_DATA_LENGTH) {
      incoming_data[data_index] = serialBuff;       //store input char at IDX to IncomingData
      data_index++;                                //inc data index
    }
  }

  if(SERIAL_RECIEVED){onSerial();}
}

void onSerial(){
    //Check if user wants to move cursor, and which direction
    if(!strcmp(incoming_data,"r")){top_display.cursor_index += 1;}
    else if(!strcmp(incoming_data,"l")){top_display.cursor_index -= 1;}

    if(!strcmp(incoming_data,"s")){  //has the select button been pressed
      top_display.smallstatus_mask[top_display.cursor_index] ^= true;
    }

    //Render the top display
    SERIAL_RECIEVED = false;
    display.clearDisplay();
    drawDisplay(top_display);
    display.display();
}

void loop() {

}
