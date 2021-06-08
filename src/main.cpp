#include <Arduino.h>
#include <Encoder.h>
#include <Control_Surface.h>

#include <LedCCPotentiometer.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

#define ENCODER_DO_NOT_USE_INTERRUPTS


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
int8_t hoverSelect = currentSelect;
uint32_t lastHoverSelect = 0;
bool displayNeedsUpdate = false;
bool displayEnabled = false;
bool isConnected = false;


#define RED_LED_PIN 4
#define RED_LED_VALUE 40
#define YELLOW_LED_PIN 3
#define YELLOW_LED_VALUE 80

USBMIDI_Interface midiUsb;

struct YellowLedUpdater : LedUpdater {
    void update(uint8_t value) override {
        uint8_t v = map(max(0, min(128, value)), 0, 128, 0, YELLOW_LED_VALUE);
        analogWrite(YELLOW_LED_PIN, v);
    }
} ledUpdater = {};


//<editor-fold desc="Multiplexers">
CD74HC4067 muxLeft = {A18, {33, 34, 35, 36}};

CD74HC4067 muxCenter = {A0, {38, 39, 32, 31}};

CD74HC4067 muxRight = {A9, {15, 16, 17, 0}};
//</editor-fold>

//<editor-fold desc="Channels">
__attribute__((unused)) LedCCPotentiometer channel1EQs[] = {
        {muxLeft.pin(4), {MIDI_CC::Channel_Volume,               CHANNEL_1},  ledUpdater},
        {muxLeft.pin(3), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_12}, ledUpdater},
        {muxLeft.pin(2), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_12}, ledUpdater},
        {muxLeft.pin(1), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_12}, ledUpdater},
        {muxLeft.pin(0), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_12}, ledUpdater},
};
__attribute__((unused)) LedCCPotentiometer channel2EQs[] = {
        {muxLeft.pin(9), {MIDI_CC::Channel_Volume,               CHANNEL_2},  ledUpdater},
        {muxLeft.pin(5), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_13}, ledUpdater},
        {muxLeft.pin(6), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_13}, ledUpdater},
        {muxLeft.pin(7), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_13}, ledUpdater},
        {muxLeft.pin(8), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_13}, ledUpdater},
};
__attribute__((unused)) LedCCPotentiometer channel3EQs[] = {
        {muxLeft.pin(14), {MIDI_CC::Channel_Volume,               CHANNEL_3},  ledUpdater},
        {muxLeft.pin(10), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_14}, ledUpdater},
        {muxLeft.pin(11), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_14}, ledUpdater},
        {muxLeft.pin(12), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_14}, ledUpdater},
        {muxLeft.pin(13), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_14}, ledUpdater},
};
__attribute__((unused)) LedCCPotentiometer channel4EQs[] = {
        {muxLeft.pin(15),  {MIDI_CC::Channel_Volume,               CHANNEL_4},  ledUpdater},
        {muxCenter.pin(0), {MIDI_CC::General_Purpose_Controller_1, CHANNEL_15}, ledUpdater},
        {muxCenter.pin(1), {MIDI_CC::General_Purpose_Controller_2, CHANNEL_15}, ledUpdater},
        {muxCenter.pin(2), {MIDI_CC::General_Purpose_Controller_3, CHANNEL_15}, ledUpdater},
        {muxCenter.pin(3), {MIDI_CC::General_Purpose_Controller_4, CHANNEL_15}, ledUpdater},
};
//</editor-fold>

//<editor-fold desc="Columns">
__attribute__((unused)) LedCCPotentiometer column1[] = {
        {muxCenter.pin(5), {MIDI_CC::Sound_Controller_1, CHANNEL_12}, ledUpdater},
        {muxCenter.pin(6), {MIDI_CC::Sound_Controller_2, CHANNEL_12}, ledUpdater},
        {muxCenter.pin(7), {MIDI_CC::Sound_Controller_3, CHANNEL_12}, ledUpdater},
        {muxCenter.pin(8), {MIDI_CC::Sound_Controller_4, CHANNEL_12}, ledUpdater},

};

__attribute__((unused)) LedCCPotentiometer column2[] = {
        {muxRight.pin(2),   {MIDI_CC::Sound_Controller_1, CHANNEL_13}, ledUpdater},
        {muxCenter.pin(13), {MIDI_CC::Sound_Controller_2, CHANNEL_13}, ledUpdater},
        {muxCenter.pin(14), {MIDI_CC::Sound_Controller_3, CHANNEL_13}, ledUpdater},
        {muxCenter.pin(15), {MIDI_CC::Sound_Controller_4, CHANNEL_13}, ledUpdater},
};

__attribute__((unused)) LedCCPotentiometer column3[] = {
        {muxRight.pin(0), {MIDI_CC::Sound_Controller_1, CHANNEL_14}, ledUpdater},
        {muxRight.pin(1), {MIDI_CC::Sound_Controller_2, CHANNEL_14}, ledUpdater},
        {muxRight.pin(3), {MIDI_CC::Sound_Controller_3, CHANNEL_14}, ledUpdater},
        {muxRight.pin(5), {MIDI_CC::Sound_Controller_4, CHANNEL_14}, ledUpdater},

};

__attribute__((unused)) LedCCPotentiometer column4[] = {
        {muxRight.pin(9),  {MIDI_CC::Sound_Controller_1, CHANNEL_15}, ledUpdater},
        {muxRight.pin(10), {MIDI_CC::Sound_Controller_2, CHANNEL_15}, ledUpdater},
        {muxRight.pin(11), {MIDI_CC::Sound_Controller_3, CHANNEL_15}, ledUpdater},
        {muxRight.pin(12), {MIDI_CC::Sound_Controller_4, CHANNEL_15}, ledUpdater},

};
//</editor-fold>

//<editor-fold desc="Bank Section">
Bank<DeviceSelect::AMOUNT> deviceBank(8);

LedBankable::LedCCPotentiometer deviceSection[] = {
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxCenter.pin(9),  {MIDI_CC::Sound_Controller_5,           CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxCenter.pin(10), {MIDI_CC::Sound_Controller_6,           CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxCenter.pin(11), {MIDI_CC::Sound_Controller_7,           CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxCenter.pin(12), {MIDI_CC::Sound_Controller_8,           CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxRight.pin(6),   {MIDI_CC::Sound_Controller_9,           CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxRight.pin(7),   {MIDI_CC::Sound_Controller_10,          CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxRight.pin(14),  {MIDI_CC::General_Purpose_Controller_5, CHANNEL_12}, ledUpdater},
        {{deviceBank, BankType::CHANGE_ADDRESS}, muxRight.pin(15),  {MIDI_CC::General_Purpose_Controller_6, CHANNEL_12}, ledUpdater},
};
//</editor-fold>


//<editor-fold desc="Declarations">
void setupDisplay();

void showConnected();

void updateDisplay();

void handleClick();

void handleLeft();

void handleRight();

struct ConnectedCallback : MIDI_Callbacks {
    void onChannelMessage(Parsing_MIDI_Interface &interface) override {
        isConnected = true;
    }

} connectedCallback = {};
//</editor-fold>





void setup() {
    setupDisplay();

    encButton.attach(ENC_BUTTON_PIN, INPUT_PULLUP);
    encButton.interval(5);
    encButton.setPressedState(LOW);

//    pinMode(RED_LED_PIN, OUTPUT);
//    pinMode(YELLOW_LED_PIN, OUTPUT);

    midiUsb.setCallbacks(connectedCallback);
    Control_Surface.begin(); // Initialize Control Surface

    encDebounce = millis();
}

void loop() {
    Control_Surface.loop(); // Update the Control Surface

    if (isConnected) {
        showConnected();
        midiUsb.setCallbacks(nullptr);
//        midiUsb.setCallbacks(midiCallbacks);
        isConnected = false;
        displayNeedsUpdate = true;
    }

    encButton.update();
    if (encButton.pressed()) {
        handleClick();
    }

    long newEncPos = enc.read();

    if (newEncPos != encPos && (millis() - encDebounce) >= 150) {
        if (newEncPos < encPos) {
            handleLeft();
        } else {
            handleRight();
        }
        uint32_t t = millis();
        lastHoverSelect = t;
        encDebounce = t;
    }
    encPos = newEncPos;

    if (hoverSelect != currentSelect && millis() - lastHoverSelect >= 2000) {
        hoverSelect = currentSelect;
        displayNeedsUpdate = true;
    }

    if (displayEnabled && displayNeedsUpdate) {
        updateDisplay();
    }
}

void updateDisplay() {
    displayNeedsUpdate = false;
    oled.clear();
    oled.set1X();
    oled.setRow(2);
    oled.set2X();
    switch (hoverSelect) {
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
            oled.print(hoverSelect);
            break;
    }
    if (hoverSelect == currentSelect) {
        oled.print(" <");
    }
}


void handleClick() {
    deviceBank.select(hoverSelect);
    currentSelect = hoverSelect;
    displayNeedsUpdate = true;
}

void handleLeft() {
    hoverSelect -= 1;
    if (hoverSelect < 0) {
        hoverSelect = DeviceSelect::AMOUNT - 1;
    }
    displayNeedsUpdate = true;
}

void handleRight() {
    hoverSelect += 1;
    if (hoverSelect >= DeviceSelect::AMOUNT) {
        hoverSelect = 0;
    }
    displayNeedsUpdate = true;
}

void setupDisplay() {
    Wire.begin();
    Wire.setClock(400000L);
    oled.begin(&Adafruit128x32, I2C_ADDRESS);
    oled.setFont(Adafruit5x7);
    oled.clear();
    oled.set1X();
    oled.setRow(2);
    oled.set2X();
    oled.write("TeensyMidi");

    displayEnabled = true;
}

void showConnected() {
    oled.clear();
    oled.set1X();
    oled.setRow(2);
    oled.set2X();
    oled.write("Connected");
    delay(2000);
}