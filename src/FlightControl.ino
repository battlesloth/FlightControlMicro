
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// multiplexers
#define TCA_1_ADDR 0x70
#define TCA_2_ADDR 0x71

#define NEO_PIXEL_PIN 6
 
// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel neoPix = Adafruit_NeoPixel(6, NEO_PIXEL_PIN, NEO_RGB + NEO_KHZ800);

struct ButtonOptions{
    uint8_t id;
    uint8_t pixelId;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    char line1[11];
    char line2[11];
    
    ButtonOptions(
    uint8_t i,
    uint8_t pId,
    uint8_t r = 0,
    uint8_t g = 0,
    uint8_t b = 0){
        id = i;
        pixelId = pId;
        red = r;
        green = g;
        blue = b;
    }
};

struct ButtonState{
    uint8_t id;
    
};

// pixelId of 99 means no pixel for this button
ButtonOptions buttonOptions[12] = {
    {0, 0},
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 4},
    {5, 5},
    {6, 99},
    {7, 99},
    {8, 99},
    {9, 99},
    {10, 99},
    {11, 99},
};

uint8_t brightness;

const int btn1_upPin = 2;
const int btn1_downPin = 3;
int btn1_upState = HIGH;
int btn1_downState = HIGH;
int last_btn1_upState = HIGH;
int last_btn1_downState = HIGH;

const int btn2_upPin = 4;
const int btn2_downPin = 5;
int btn2_upState = HIGH;
int btn2_downState = HIGH;
int last_btn2_upState = HIGH;
int last_btn2_downState = HIGH;

unsigned long debounceDelay = 50;

char inputBuffer[64];
bool receivingInput = false;
int indexCounter = 0;
char line1Buffer[11];
char line2Buffer[11];

char strVals[4][11] = {"ABCDEFGHIJ", "1 DOWN", "ABCDEFG", "2 DOWN"};
char empty[2];

uint8_t displayCount = 8;

Adafruit_SSD1306 displayBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// select multipled I2C channel
bool channelSelect(uint8_t channel){
    
    if (channel > displayCount) return false;

    if (channel > 7){
        channel -= 8;
        disableTca(TCA_1_ADDR);
        tcaSelect(TCA_2_ADDR, channel);
    } else {
        disableTca(TCA_2_ADDR);
        tcaSelect(TCA_1_ADDR, channel);
    }
}

// select channel on I2C multiplexer
void tcaSelect(uint8_t tcaId, uint8_t channel){
    
    Wire.beginTransmission(tcaId);
    Wire.write(1 << channel);
    Wire.endTransmission();
} 

// disable multiplexer
void disableTca(uint8_t tcaId){
    
    Wire.beginTransmission(tcaId);
    Wire.write(0);
    Wire.endTransmission();
}

void setup() {

    Serial.begin(115200);

    neoPix.begin();

    Wire.begin();

    pinMode(btn1_upPin, INPUT_PULLUP);
    pinMode(btn1_downPin, INPUT_PULLUP);
    pinMode(btn2_upPin, INPUT_PULLUP);
    pinMode(btn2_downPin, INPUT_PULLUP);

    for (int8_t i = 0; i < 8; i++){
        channelSelect(i);
        
        if (!displayBuffer.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
            Serial.println(F("SSD1306 0 allocation failed"));
        }

    }


    char welcome[11] = "07 CMDR";
    char line0[11] = "Screen 0";
    char line1[11] = "Screen 1";
    char line2[11] = "Screen 2";
    char line3[11] = "Screen 3";
    char line4[11] = "Screen 4";
    char line5[11] = "Screen 5";
    char line6[11] = "Screen 6";
    char line7[11] = "Screen 7";

    drawText(welcome, line0, 0);
    delay(100);
    drawText(welcome, line1, 1);
    delay(100);
    drawText(welcome, line2, 2);
    delay(100);
    drawText(welcome, line3, 3);
    delay(100);
    drawText(welcome, line4, 4);
    delay(100);
    drawText(welcome, line5, 5);
    delay(100);
    drawText(welcome, line6, 6);
    delay(100);
    drawText(welcome, line7, 7);
}

void loop(){

    if (!receivingInput){
        int reading1u = digitalRead(btn1_upPin);
        int reading1d = digitalRead(btn1_downPin);
        int reading2u = digitalRead(btn2_upPin);
        int reading2d = digitalRead(btn2_downPin);

        if (reading1u == last_btn1_upState &&
            reading1d == last_btn1_downState &&
            reading2u == last_btn2_upState &&
            reading2d == last_btn2_downState)
        {
            return;
        }

        delay(debounceDelay);

        reading1u = digitalRead(btn1_upPin);
        reading1d = digitalRead(btn1_downPin);
        reading2u = digitalRead(btn2_upPin);
        reading2d = digitalRead(btn2_downPin);

       //if (reading1u == LOW){
       //    channelSelect(1);
       //    drawText(strVals[0], empty, 0);
       //} else if (reading1d == LOW){
       //    channelSelect(1);
       //    drawText(strVals[1], empty, 0);
       //} else if (reading2u == LOW){
       //    channelSelect(8);
       //    drawText(strVals[2], empty, 1);
       //} else if (reading2d == LOW){
       //    channelSelect(8);
       //    drawText(strVals[3], empty, 1);
       //} else {
       //    return;
       //}
    }
}




// receive serial communication
void serialEvent(){
    
    while(Serial.available()){
    
        if (!receivingInput){
            memset(inputBuffer, 0, 64);
            receivingInput = true;
            indexCounter = 0;
        }
        char inChar = (char)Serial.read();

        inputBuffer[indexCounter] = inChar;
        indexCounter++;

        if (inChar == '\n'){
            receivingInput = false;  
            parseSerialMessage(inputBuffer);         
        }
    }
    
}

// parse serial input for setting a controls value
void parseSerialMessage(char buffer[]){

         // P is the header for Program
        if (buffer[0] != 'P'){
            Serial.println(F("Input format error"));

            return;
        }

        switch (buffer[1])
        {
            case 'U':
                ProcessUpdate(buffer);
                break;
            default:
                return;
        }     
}

void ProcessUpdate(char buffer[]){
    
    char num[3];
    char temp[11];
    int placeCount = 2;
    int lineLen = 0;
    
    // button ids are A - L (1 - 12)
    uint8_t btnId = buffer[placeCount] - 'A';
    placeCount++;

    num[0] = buffer[placeCount];
    placeCount++;
    num[1] = buffer[placeCount];
    placeCount++;
    num[2] = '\0';

    lineLen = atoi(num);  

    memset(temp, 0, 11);
    strncpy(temp, buffer + placeCount, lineLen);       
    memcpy(line1Buffer, temp, 11);
    
    placeCount += lineLen;

    num[0] = buffer[placeCount];
    placeCount++;
    num[1] = buffer[placeCount];
    placeCount++;
    num[2] = '\0';

    lineLen = atoi(num);  

    memset(temp, 0, 11);
    strncpy(temp, buffer + placeCount, lineLen);       
    memcpy(line2Buffer, temp, 11);

    channelSelect(btnId);
    //drawText(line1Buffer, line2Buffer, btnId);
}

void drawText(char line1[], char line2[], int8_t channel){

    channelSelect(channel);
    //if (!channelSelect(id)){
    //    return;
    //}

    //Adafruit_SSD1306 display = displays[id];          

    displayBuffer.clearDisplay();  
    
    displayBuffer.setTextColor(WHITE);

    // line is 7 chars or less and line 2 is empty
    // then we will display it as large text
    if (line1[7] == 0 && line2[0] == 0){
    
        displayBuffer.setTextSize(3);
        int offset = getOffSet(line1, true) + 2;
        displayBuffer.setCursor(offset, 5);
        displayBuffer.print(line1);

    } else if (line2[0] == 0){

        displayBuffer.setTextSize(2);
        int offset = getOffSet(line1, false) + 5;
        displayBuffer.setCursor(offset, 8);
        displayBuffer.print(line1);

    } else {

        displayBuffer.setTextSize(2);

        int offset1 = getOffSet(line1, false) + 5;
        displayBuffer.setCursor(offset1, 0);
        displayBuffer.print(line1);
        
        int offset2 = getOffSet(line2, false) + 5;
        displayBuffer.setCursor(0, 16);
        displayBuffer.setCursor(offset2, 16);
        displayBuffer.print(line2);

    }
   
    displayBuffer.display();
}

// get offset to center text
int getOffSet(char val[], bool isLarge){
    // size 2 is 12 pixel
    // size 3 is 18 pixels

    if (isLarge){
        for (int i = 6; i > -1; i--){
            if (val[i] != 0){
                return (6 - i) * 9;        
            }
        }
    } else {
        for (int i = 9; i > -1; i--){
            if (val[i] != 0){
                return (9 - i) * 6;        
            }
        }
    }
}
