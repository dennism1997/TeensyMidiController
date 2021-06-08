//
// Created by dennis on 04-06-21.
//

#ifndef ARDUINO_MIDI_CONTROLLER_LEDUPDATER_H
#define ARDUINO_MIDI_CONTROLLER_LEDUPDATER_H

class LedUpdater {
public:
    virtual void update(uint8_t value) { (void) value; };

    virtual ~LedUpdater() = default;
};

#endif //ARDUINO_MIDI_CONTROLLER_LEDUPDATER_H
