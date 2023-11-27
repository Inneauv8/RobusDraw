#ifndef SD_STATE_H
#define SD_STATE_H

#include <Arduino.h>  // Include for strcmp function
#include <SD.h>

namespace SDState {

    enum SDState {
        UNKNOWN,
        NOT_PRESENT,
        PRESENT
    };

    void registerCard(uint8_t chipSelect);

    void refresh();

    void setListener(void (*_listener)(SDState));

    SDState getCardState();

    bool isCardPresent();
    bool isCardNotPresent();
    bool isCardStateUnknown();

    namespace {
        extern uint8_t chipSelectPin;
        extern SDState state;

        extern void (*listener)(SDState);
    }
}

#endif // SD_STATE_H
