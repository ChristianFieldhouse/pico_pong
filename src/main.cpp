#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PAD_WIDTH 10
#define PAD_OFFSET 2

#define right0 14
#define left0 16

#define right1 1
#define left1 0

int score0 = 0;
int score1 = 0;

struct ball{
  ball(){
    x = SCREEN_WIDTH / 2;
    y = 0;
    vx = 0.5;
    vy = 0.5;
  }
  float x;
  float y;
  float vx;
  float vy;
  void step(float p0, float p1, int pv0, int pv1){
    if ((x < PAD_OFFSET + 1) && (y > p0 && y < (p0 + PAD_WIDTH))){
        vx = -vx;
    }
    if ((x > SCREEN_WIDTH - PAD_OFFSET - 2) && (y > p1 && y < (p1 + PAD_WIDTH))){
        vx = -vx;
    }
    if ((y < 0) || (y > SCREEN_HEIGHT)){
      vy = -vy;
    }
    x += vx;
    y += vy;
  }
  uint8_t x8(){
    float x_clipped = x;
    if (x_clipped < 0){
      x_clipped = 0.f;
    }
    if (x_clipped > SCREEN_WIDTH - 1){
      x_clipped = SCREEN_WIDTH - 1;
    }
    return uint8_t(x_clipped);
  }
  uint8_t y8(){
    float y_clipped = y;
    if (y_clipped < 0){
      y_clipped = 0.f;
    }
    if (y_clipped > SCREEN_HEIGHT - 1){
      y_clipped = SCREEN_HEIGHT - 1;
    }
    return uint8_t(y_clipped);
  }
};
ball b;
float pad0 = SCREEN_HEIGHT/2;
float pad1 = SCREEN_HEIGHT/2;
void drawPong(uint8_t p0, uint8_t p1, uint8_t x, uint8_t y);
void drawEnd();

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { //Ive changed the address //already chill
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

  drawPong(0, 0, 0, 0);

  pinMode(left0, INPUT_PULLDOWN);
  pinMode(right0, INPUT_PULLDOWN);
  pinMode(left1, INPUT_PULLDOWN);
  pinMode(right1, INPUT_PULLDOWN);
}

float pad_vel = 1.5;
void loop() {
  int pv0 = int(digitalRead(left0)) - int(digitalRead(right0));
  int pv1 = int(digitalRead(left1)) - int(digitalRead(right1));
  if (b.x + b.vx < PAD_OFFSET + 1){
    if (pv0/b.vy > 0){
      b.vx *= 1.2;
      b.vy *= 1.2;
      pad_vel *= 1.2;
    }
    if (pv0/b.vy < 0){
      b.vy *= -1;
    }
  }
  if (b.x + b.vx > SCREEN_WIDTH - PAD_OFFSET - 2){
    if (pv1/b.vy > 0){
      b.vx *= 1.2;
      b.vy *= 1.2;
      pad_vel *= 1.2;
    }
    if (pv1/b.vy < 0){
      b.vy *= -1;
    }
  }
  b.step(pad0, pad1, pv0, pv1);
  pad0 += pad_vel * pv0;
  pad1 += pad_vel * pv1;

  if (b.x < 0){
    ++score1;
    drawEnd();
    pad0 = SCREEN_HEIGHT/2;
    pad1 = SCREEN_HEIGHT/2;
    b = ball();
    pad_vel = 1.5;
  }
  if (b.x > SCREEN_WIDTH){
    ++score0;
    drawEnd();
    pad0 = SCREEN_HEIGHT/2;
    pad1 = SCREEN_HEIGHT/2;
    b = ball();
    pad_vel = 1.5;
  }
  drawPong(pad0, pad1, b.x8(), b.y8());
}

void drawPong(uint8_t p0, uint8_t p1, uint8_t x, uint8_t y) {
  display.clearDisplay(); // Clear display buffer
  display.drawLine(PAD_OFFSET, max(p0, 0), PAD_OFFSET, p0 + PAD_WIDTH, SSD1306_WHITE);
  display.drawLine(display.width() - 1 - PAD_OFFSET, max(p1, 0), display.width() - 1 - PAD_OFFSET, p1 + PAD_WIDTH, SSD1306_WHITE);
  display.drawPixel(x, y, SSD1306_WHITE);
  display.display();
}

void drawEnd() {
  display.clearDisplay();

  display.setTextSize(8);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, (SCREEN_HEIGHT - 8 * 7) / 2);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.write(score0 + 48);

  display.setCursor(SCREEN_WIDTH - 8 * 6, (64 - 8 * 7) / 2);
  display.write(score1 + 48);

  display.display();
  delay(2000);
}