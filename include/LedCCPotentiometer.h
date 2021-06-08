//
// Created by dennis on 04-06-21.
//

#ifndef ARDUINO_MIDI_CONTROLLER_LEDCCPOTENTIOMETER_H
#define ARDUINO_MIDI_CONTROLLER_LEDCCPOTENTIOMETER_H

#include <Control_Surface.h>
#include <LedUpdater.h>
#include <Banks/BankableAddresses.hpp>

template<class Sender>
class LedMIDIFilteredAnalogAddressable : public MIDIOutputElement {
protected:
    /**
     * @brief   Construct a new MIDIFilteredAnalog.
     *
     * @param   analogPin
     *          The analog input pin with the wiper of the potentiometer
     *          connected.
     * @param   address
     *          The MIDI address to send to.
     * @param   sender
     *          The MIDI sender to use.
     */
    LedMIDIFilteredAnalogAddressable(pin_t analogPin, const MIDIAddress &address,
                                     const Sender &sender)
            : filteredAnalog{analogPin}, address{address}, sender(sender) {}

public:
    void begin() override {}

    void update() override {
        if (filteredAnalog.update())
            sender.send(filteredAnalog.getValue(), address);
    }

    /**
     * @brief   Specify a mapping function that is applied to the raw
     *          analog value before sending.
     *
     * @param   fn
     *          A function pointer to the mapping function. This function
     *          should take the filtered analog value of @f$ 16 -
     *          \mathrm{ANALOG\_FILTER\_SHIFT\_FACTOR} @f$ bits as a parameter,
     *          and should return a value in the same range.
     *
     * @see     FilteredAnalog::map
     */
    void map(MappingFunction fn) { filteredAnalog.map(fn); }

    /// Invert the analog value.
    void invert() { filteredAnalog.invert(); }

    /**
     * @brief   Get the raw value of the analog input (this is the value
     *          without applying the filter or the mapping function first).
     */
    analog_t getRawValue() const { return filteredAnalog.getRawValue(); }

    /**
     * @brief   Get the value of the analog input (this is the value after first
     *          applying the mapping function).
     */
    analog_t getValue() const { return filteredAnalog.getValue(); }

protected:
    AH::FilteredAnalog<Sender::precision()> filteredAnalog;
    const MIDIAddress address;

public:
    Sender sender;
};

class LedCCPotentiometer : public LedMIDIFilteredAnalogAddressable<ContinuousCCSender> {
public:
/**
 * @brief   Create a new CCPotentiometer object with the given analog pin,
 *          controller number and channel.
 *
 * @param   analogPin
 *          The analog input pin to read from.
 * @param   address
 *          The MIDI address containing the controller number [0, 119],
 *          channel [CHANNEL_1, CHANNEL_16], and optional cable number
 *          [CABLE_1, CABLE_16].
 */
    LedCCPotentiometer(pin_t analogPin, const MIDIAddress &address, LedUpdater &updater)
            : LedMIDIFilteredAnalogAddressable(analogPin, address, {}), updater(updater) {
    }


    void update() override {
        if (filteredAnalog.update()) {
            analog_t value = filteredAnalog.getValue();
            sender.send(value, address);
            updater.update(value);
        }

    }
private:
    LedUpdater& updater;
};

#include <Banks/BankAddresses.hpp>

namespace LedBankable {
#include <Banks/BankAddresses.hpp>

    template <class BankAddress, class Sender>
    class LedMIDIFilteredAnalogAddressable : public MIDIOutputElement {
    protected:
        /**
         * @brief   Construct a new MIDIFilteredAnalog.
         *
         * @param   bankAddress
         *          The bankable MIDI address to send to.
         * @param   analogPin
         *          The analog input pin with the wiper of the potentiometer
         *          connected.
         * @param   sender
         *          The MIDI sender to use.
         */
        LedMIDIFilteredAnalogAddressable(BankAddress bankAddress,
                                      pin_t analogPin, const Sender &sender)
                : address{bankAddress}, filteredAnalog{analogPin}, sender(sender) {}

    public:
        void begin() override {}
        void update() override {
            if (filteredAnalog.update())
                sender.send(filteredAnalog.getValue(), address.getActiveAddress());
        }

        /**
         * @brief   Specify a mapping function that is applied to the raw
         *          analog value before sending.
         *
         * @param   fn
         *          A function pointer to the mapping function. This function
         *          should take the filtered analog value of @f$ 16 -
         *          \mathrm{ANALOG\_FILTER\_SHIFT\_FACTOR} @f$ bits as a parameter,
         *          and should return a value in the same range.
         *
         * @see     FilteredAnalog::map
         */
        void map(MappingFunction fn) { filteredAnalog.map(fn); }

        void invert() { filteredAnalog.invert(); }


        analog_t getRawValue() const { return filteredAnalog.getRawValue(); }

        analog_t getValue() const { return filteredAnalog.getValue(); }

    protected:
        BankAddress address;
        AH::FilteredAnalog<Sender::precision()> filteredAnalog;

    public:
        Sender sender;
    };

    class SingleAddress : public OutputBankableMIDIAddress {
    public:
        SingleAddress(BaseOutputBankConfig config, MIDIAddress address)
                : OutputBankableMIDIAddress{config}, address{address} {}

        MIDIAddress getBaseAddress() const { return address; }

        MIDIAddress getActiveAddress() const {
            return getBaseAddress() + getAddressOffset();
        }

    private:
        MIDIAddress address;
    };

    class LedCCPotentiometer : public LedMIDIFilteredAnalogAddressable<SingleAddress, ContinuousCCSender> {
    public:
        LedCCPotentiometer(OutputBankConfig<> config, pin_t analogPin, const MIDIAddress &address, LedUpdater &updater)
                : LedMIDIFilteredAnalogAddressable(SingleAddress{config, address}, analogPin, {}), updater(updater) {
        }


        void update() override {
            if (filteredAnalog.update()) {
                analog_t value = filteredAnalog.getValue();
                sender.send(value, address.getActiveAddress());
                updater.update(value);
            }

        }
    private:
        LedUpdater& updater;
    };
}

#endif //ARDUINO_MIDI_CONTROLLER_LEDCCPOTENTIOMETER_H
