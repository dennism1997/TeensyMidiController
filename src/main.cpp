#include <Arduino.h>
#include <Control_Surface.h>

#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;


USBMIDI_Interface midi;

////<editor-fold desc="Multiplexers">
//CD74HC4067 muxLeft = {A18, {33, 34, 35, 36}};
//
//CD74HC4067 muxCenter = {A0, {38, 39, 32, 31}};
//
//CD74HC4067 muxRight = {A9, {15, 16, 17, 0}};
////</editor-fold>

////<editor-fold desc="Channels">
//CCPotentiometer channel1EQs[] = {
////        {muxLeft.pin(4), {MIDI_CC::Channel_Volume,               CHANNEL_12}},
//        {muxLeft.pin(3), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_12}},
//        {muxLeft.pin(2), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_13}},
//        {muxLeft.pin(1), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_14}},
//        {muxLeft.pin(0), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_15}},
//};
//CCPotentiometer channel2EQs[] = {
////        {muxLeft.pin(6),  {MIDI_CC::Channel_Volume,               CHANNEL_13}},
//        {muxLeft.pin(5), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_13}},
//        {muxLeft.pin(6), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_13}},
//        {muxLeft.pin(7), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_13}},
//        {muxLeft.pin(8), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_13}},
//};
//CCPotentiometer channel3EQs[] = {
////        {muxLeft.pin(5), {MIDI_CC::Channel_Volume,               CHANNEL_14}},
//        {muxLeft.pin(10), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_14}},
//        {muxLeft.pin(11), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_14}},
//        {muxLeft.pin(12), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_14}},
//        {muxLeft.pin(13), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_14}},
//};
//CCPotentiometer channel4EQs[] = {
////        {muxCenter.pin(9),  {MIDI_CC::Channel_Volume,               CHANNEL_15}},
//        {muxCenter.pin(0), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_15}},
//        {muxCenter.pin(1), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_15}},
//        {muxCenter.pin(2), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_15}},
//        {muxCenter.pin(3), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_15}},
//};
////</editor-fold>

////<editor-fold desc="Columns">
//CCPotentiometer column1[] = {
//        {muxCenter.pin(5),  {MIDI_CC::Sound_Controller_1, CHANNEL_12}},
//        {muxCenter.pin(6),  {MIDI_CC::Sound_Controller_1, CHANNEL_12}},
//        {muxCenter.pin(7),  {MIDI_CC::Sound_Controller_2, CHANNEL_12}},
//        {muxCenter.pin(8),  {MIDI_CC::Sound_Controller_3, CHANNEL_12}},
//        {muxCenter.pin(9),  {MIDI_CC::Sound_Controller_4, CHANNEL_12}},
//        {muxCenter.pin(10), {MIDI_CC::Sound_Controller_5, CHANNEL_12}},
//};
//
//CCPotentiometer column2[] = {
//        {muxCenter.pin(11), {MIDI_CC::Sound_Controller_1, CHANNEL_13}},
//        {muxRight.pin(2),   {MIDI_CC::Sound_Controller_2, CHANNEL_13}},
//        {muxCenter.pin(12), {MIDI_CC::Sound_Controller_3, CHANNEL_13}},
//        {muxCenter.pin(13), {MIDI_CC::Sound_Controller_4, CHANNEL_13}},
//        {muxCenter.pin(14), {MIDI_CC::Sound_Controller_5, CHANNEL_13}},
//        {muxCenter.pin(15), {MIDI_CC::Sound_Controller_6, CHANNEL_13}},
//};
//
//CCPotentiometer column3[] = {
//        {muxRight.pin(0), {MIDI_CC::Sound_Controller_1, CHANNEL_14}},
//        {muxRight.pin(1), {MIDI_CC::Sound_Controller_2, CHANNEL_14}},
//        {muxRight.pin(3), {MIDI_CC::Sound_Controller_3, CHANNEL_14}},
//        {muxRight.pin(5), {MIDI_CC::Sound_Controller_4, CHANNEL_14}},
//        {muxRight.pin(6), {MIDI_CC::Sound_Controller_5, CHANNEL_14}},
//        {muxRight.pin(7), {MIDI_CC::Sound_Controller_6, CHANNEL_14}},
//};
//
//CCPotentiometer column4[] = {
//        {muxRight.pin(9),  {MIDI_CC::Sound_Controller_2, CHANNEL_15}},
//        {muxRight.pin(10), {MIDI_CC::Sound_Controller_3, CHANNEL_15}},
//        {muxRight.pin(11), {MIDI_CC::Sound_Controller_4, CHANNEL_15}},
//        {muxRight.pin(12), {MIDI_CC::Sound_Controller_5, CHANNEL_15}},
//        {muxRight.pin(14), {MIDI_CC::Sound_Controller_6, CHANNEL_15}},
//        {muxRight.pin(15), {MIDI_CC::Sound_Controller_6, CHANNEL_15}},
//};
//
//
////</editor-fold>

#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

Encoder enc(2, 1);
long encPos = 0;
uint32_t encDebounce;

#include <Bounce2.h>

Bounce2::Button encButton = Bounce2::Button();
#define ENC_BUTTON_PIN 5

enum DeviceSelect {
    selectedDevice = 0, Master = 1, Reverb = 2, AMOUNT = 3
};
int8_t currentSelect = DeviceSelect::selectedDevice;
bool displayNeedsUpdate = false;
bool displayEnabled = false;

#define RED_LED_PIN 4
#define RED_LED_VALUE 60
#define YELLOW_LED_PIN 3
#define YELLOW_LED_VALUE 120


void setupDisplay();

void updateDisplay();

void handleClick();

void handleLeft();

void handleRight();


void setup() {
    setupDisplay();

    encButton.attach(ENC_BUTTON_PIN, INPUT_PULLUP);
    encButton.interval(5);
    encButton.setPressedState(LOW);

    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);


    Control_Surface.begin(); // Initialize Control Surface
    encDebounce = millis();
}

void loop() {
    Control_Surface.loop(); // Update the Control Surface

    encButton.update();
    if (encButton.pressed()) {
        handleClick();
    }

    long newEncPos = enc.read();

    if (newEncPos != encPos && (millis() - encDebounce) > 100) {
        if (newEncPos < encPos) {
            handleLeft();
        } else {
            handleRight();
        }
        encDebounce = millis();
    }
    encPos = newEncPos;

    if (displayEnabled && displayNeedsUpdate) {
        updateDisplay();
    }
}

void updateDisplay() {
    displayNeedsUpdate = false;
    oled.clear();
    oled.set1X();
    oled.setRow(1);
    switch (currentSelect) {
        case DeviceSelect::selectedDevice: {
            oled.print("Device");
            break;
        }
        case DeviceSelect::Master: {
            oled.print("Master");
            break;
        }
        case DeviceSelect::Reverb: {
            oled.print("Reverb");
            break;
        }
        case AMOUNT:
            oled.print("amt");
            break;
        default:
            oled.print(currentSelect);
            break;
    }
}


void handleClick() {
    analogWrite(RED_LED_PIN, RED_LED_VALUE);
    analogWrite(YELLOW_LED_PIN, YELLOW_LED_VALUE);
}

void handleLeft() {
    currentSelect -= 1;
    if (currentSelect < 0) {
        currentSelect = DeviceSelect::AMOUNT - 1;
    }
    displayNeedsUpdate = true;

    analogWrite(RED_LED_PIN, 0);
    analogWrite(YELLOW_LED_PIN, YELLOW_LED_VALUE);
}

void handleRight() {
    currentSelect += 1;
    if (currentSelect >= DeviceSelect::AMOUNT) {
        currentSelect = 0;
    }
    displayNeedsUpdate = true;
    analogWrite(RED_LED_PIN, RED_LED_VALUE);
    analogWrite(YELLOW_LED_PIN, 0);
}

void setupDisplay() {
    Wire.begin();
    Wire.setClock(400000L);
    oled.begin(&Adafruit128x32, I2C_ADDRESS);
    oled.setFont(Adafruit5x7);

    displayEnabled = true;
}
