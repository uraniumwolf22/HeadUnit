

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>

#define BOOT_DELAY 1000
#define SKIP_BOOT_SEQ false
#define NUM_DISPLAYS 2
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MAX_DATA_LENGTH 200 //set maximum data length for serial
#define NUM_MODES 6
#define NUM_STATS 8

#define OLED_RESET -1

#define ENC_A 33  //encoder pin A
#define ENC_B 32
#define ENC_SW

#define DEBUG_MODE true
#define VOLUME_ENABLE



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

enum InputType {
  LEFT,
  RIGHT,
  CLICK
};

enum Volume {
  UP,
  DOWN
};

struct Display {        //struct to store display proprties
  Adafruit_SSD1306* ctx;        //I2C Display object
  char serial_address;
  DisplayMode active_mode;      //display mode that is currently being rendered
  DisplayMode display_mode;     //last status screen that was being rendered

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


struct Statistic stats[NUM_STATS] = {   //list and default values of current supported statistics
  {0x5C, "OIL", "F", 0, 100, 80, 0},
  {0x05, "COOL", "F", 0, 100, 80, 0},
  {0x0E, "ADV", "deg", 0, 100, 80, 0},
  {0x24, "AFR", "", 0, 100, 80, 0},
  {0x06, "STFT", "%", 0, 100, 80, 0},
  {0x07, "LTFT", "%", 0, 100, 80, 0},
  {0x0F, "INTK", "F", 0, 100, 80, 0},
  {0x05, "RPM", "", 0, 100, 80, 0},
};


Adafruit_SSD1306 display1(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

struct Display displays[NUM_DISPLAYS] = {
  {&display1, 0x3D, SMALLSTATUS, SMALLSTATUS, -1, {0}, {0}, {0}},
  {&display2, 0x3C, SMALLSTATUS, SMALLSTATUS, -1, {0}, {0}, {0}},
};



void drawStatMenu(Display &display) {
    const bool flipped = (display.ctx->getRotation() == 2);
    const int offset = flipped ? 0 : 16;

    display.ctx->setTextSize(1); // 7px tall (8 is ideal)

    // Draw statistic buttons
    for (int i = 0; i < min(NUM_STATS, 16); i++) {
        const int x = (i % 4) * 32;
        const int y = (i / 4) * 12 + offset;

        int text_width;
        display.ctx->getTextBounds(stats[i].name, 0, 0,NULL, NULL, &text_width, NULL);
        display.ctx->setCursor(x + (32 - text_width) / 2, y + 2);  // Center text in bounding box

        if (getSelectionMask(display)[i]) {
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

  display.ctx->setTextColor(SSD1306_WHITE);
  display.ctx->setCursor(x, y);
  display.ctx->println("BACK");

  if (display.cursor_index == -1) {
    display.ctx->drawFastHLine(x, y+9, text_width, SSD1306_WHITE);

  }
}

void drawModeMenu (Display &display) {
  const char* mode_names[NUM_MODES] = {
    "",
    "STAT SELECT",
    "VOLUME",
    "BARS (LARGE)",
    "BARS (SMALL)",
    "GRAPH"
  };

  // PLACEHOLDER
  display.ctx->setTextColor(SSD1306_WHITE);
  display.ctx->setCursor(4, 16);
  display.ctx->println("MENU");
  display.ctx->setCursor(4, 26);
  display.ctx->println(mode_names[display.cursor_index]);
}

void drawVolume (Display &display) {
  display.ctx->setTextColor(SSD1306_WHITE);
  display.ctx->setCursor(4, 0);
  display.ctx->println("VOLUME");
}

void drawBigStatus (Display &display) {
  display.ctx->setTextColor(SSD1306_WHITE);
  display.ctx->setCursor(4, 0);
  display.ctx->println("BARS (LARGE)");
}

void drawSmallStatus (Display &display) {
  display.ctx->setTextColor(SSD1306_WHITE);
  display.ctx->setCursor(4, 0);
  display.ctx->println("BARS (SMALL)");
}

void drawGraph (Display &display) {
  display.ctx->setTextColor(SSD1306_WHITE);
  display.ctx->setCursor(4, 0);
  display.ctx->println("GRAPH");
}


void drawDisplay(Display &display) {   //draw current mode onto correct display
  display.ctx->clearDisplay();
  switch (display.active_mode) {
    case MODEMENU: drawModeMenu(display); break;
    case STATMENU: drawStatMenu(display); break;
    case VOLUME: drawVolume(display); break;
    case BIGSTATUS: drawBigStatus(display); break;
    case SMALLSTATUS: drawSmallStatus(display); break;
    case GRAPH: drawGraph(display); break;
  }
  display.ctx->display();
}


bool* getSelectionMask(Display &display) {
  switch (display.display_mode) {
    case BIGSTATUS: return display.bigstatus_mask; break;
    case SMALLSTATUS: return display.smallstatus_mask; break;
    case GRAPH: return display.graph_mask; break;
    default:
      bool empty[NUM_STATS] = {false};
      return empty;
  }
  display.ctx->display();
}

int getMaskSize(bool* &mask, int size) {
  int count = 0;
  for (int i=0; i<size; i++) {
    if (mask[i]) { count++; }
  }
  return count;
}

Display* active_display = NULL;
bool selection_mode = false;
int display_cursor = 0;

void drawSelectionOverlay() {
  for (int i=0; i<NUM_DISPLAYS; i++) {
    if (i != display_cursor) {
      drawDisplay(displays[i]);
      continue;
    }

    displays[i].ctx->fillRect(0, 0, SCREEN_WIDTH, 8, SSD1306_WHITE);
    displays[i].ctx->fillRect(0, SCREEN_HEIGHT-8, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    displays[i].ctx->fillRect(0, 0, 8, SCREEN_HEIGHT, SSD1306_WHITE);
    displays[i].ctx->fillRect(SCREEN_WIDTH-8, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
    displays[i].ctx->display();
  }
}

void handleModeMenu(InputType type) {
  if (type == CLICK) {
    // Set currently selected mode
    active_display->active_mode = active_display->cursor_index;
    if (active_display->active_mode != STATMENU) {
      active_display = NULL;
    }
  }

  else {
    // Change selected mode
    // Cursor index <-> DisplayMode, exluding 0 (current mode)
    type == RIGHT ?
      active_display->cursor_index++ :
      active_display->cursor_index-- ;

    if (active_display->cursor_index > NUM_MODES-1) { active_display->cursor_index = 1; }
    else if (active_display->cursor_index < 1) { active_display->cursor_index = NUM_MODES-1; }
  }
}

void handleStatMenu(InputType type) {
  if (type == CLICK) {
    if (active_display->cursor_index == -1) {
      active_display->cursor_index = STATMENU;
      active_display->active_mode = MODEMENU;
    }
    else {
      bool* selection = getSelectionMask(*active_display);
      // Deselect if active
      if (selection[active_display->cursor_index]) {
        selection[active_display->cursor_index] = false;
      }
      // Select if inactive & maximum is not reached
      else {
        const int num_selected = getMaskSize(selection, NUM_STATS);

        // Set maximum selection sizes
        int limits[NUM_MODES] = {0};
        limits[BIGSTATUS] = 2;
        limits[SMALLSTATUS] = 4;
        limits[GRAPH] = 1;

        if (num_selected < limits[active_display->display_mode]) {
          selection[active_display->cursor_index] = true;
        }
      }

    }
  }

  else {
    // Change cursor position
    // Cursor index <-> stats index, -1 indicates "Back" button
    type == RIGHT ?
      active_display->cursor_index++ :
      active_display->cursor_index-- ;

    const int max = min(NUM_STATS, 16)-1;
    if (active_display->cursor_index > max) { active_display->cursor_index = -1; }
    else if (active_display->cursor_index < -1) { active_display->cursor_index = max; }
  }
}

void handleInput(InputType type) {
  if (active_display && !selection_mode) {
    switch (active_display->active_mode) {
      case MODEMENU: handleModeMenu(type); break;
      case STATMENU: handleStatMenu(type); break;
    }
  }
  else {
    if (selection_mode) {
      if (type == CLICK) {
        selection_mode = false;
        active_display = &displays[display_cursor];
        active_display->cursor_index = active_display->active_mode;
        active_display->display_mode = active_display->active_mode;
        active_display->active_mode = MODEMENU;
      }

      else {
        type == RIGHT ?
          display_cursor++ :
          display_cursor-- ;

        if (display_cursor > NUM_DISPLAYS-1) { display_cursor = 0; }
        else if (display_cursor < 0) { display_cursor = NUM_DISPLAYS-1; }

        drawSelectionOverlay();
      }
    }

    else {
      if (type == CLICK) {
        selection_mode = true;
        drawSelectionOverlay();
      }
      else {
        type == RIGHT ?
          handleVolume(UP) :
          handleVolume(DOWN);

        displays[0].active_mode = VOLUME;
      }
    }
  }
}

void handleVolume(Volume volume){
  if(volume == UP){
    Serial.println("1");
  }
  if(volume == DOWN){
    Serial.println("0");
  }
}

void setup() {

  Serial.begin(115200);

  displays[0].ctx->setRotation(2);

  for (int i=0; i<NUM_DISPLAYS; i++) {
    if (!displays[i].ctx->begin(SSD1306_SWITCHCAPVCC, displays[i].serial_address)) {
      Serial.println(F("SSD1306 allocation failed"));
      for (;;);
    }

    displays[i].ctx->clearDisplay();
    drawDisplay(displays[i]);
  }

}

//Yeah Yeah there are tons of comments becuse this one hurts my small brain
char incoming_data[MAX_DATA_LENGTH] = "";  //Place to store incoming serial data
bool SERIAL_RECIEVED = false;             //Place to store current state of serial input buffer

void serialEvent() {                      //runs asyncronously with main loop
  static int data_index = 0;                 // Index for storing incoming data

  while (Serial.available()) {                  //check if serial data is in buffer
    char serialbuff = (char)Serial.read();      //set input char to serial buffer

    if (serialbuff == '\n') {                   //check if serial communication frame has been terminated
      incoming_data[data_index] = '\0';             // Terminate the string
      SERIAL_RECIEVED = true;
      data_index = 0;                              //Reset data index
    } else if (data_index < MAX_DATA_LENGTH) {
      incoming_data[data_index] = serialbuff;       //store input char at IDX to IncomingData
      data_index++;                                //inc data index
    }
  }
}

void loop() {
  if(SERIAL_RECIEVED && DEBUG_MODE){
    if(!strcmp(incoming_data,"r")){ handleInput(RIGHT); }
    else if(!strcmp(incoming_data,"l")){ handleInput(LEFT); }
    else if(!strcmp(incoming_data,"s")){ handleInput(CLICK); }

    else if(!strcmp(incoming_data,"u")){ handleVolume(UP); }
    else if(!strcmp(incoming_data,"d")){ handleVolume(DOWN); }

    SERIAL_RECIEVED = false;

    if (!selection_mode) {
      for (int i=0; i<NUM_DISPLAYS; i++) {
        drawDisplay(displays[i]);
      }
    }
  }
}
