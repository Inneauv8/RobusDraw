#include "SDState.h"

/**
 * @file sd_state.h
 * @brief SD card state management functions and definitions.
 */
namespace SDState {

    /**
     * @brief Register the chip select pin for the SD card.
     * @param chipSelect The pin to be used as the chip select for the SD card.
     */
    void registerCard(uint8_t chipSelect) {
        chipSelectPin = chipSelect;
    }

    /**
     * @brief Refresh the SD card state and notify the listener if there is a change.
     * 
     * This function checks the current state of the SD card and compares it
     * with the previous state. If there is a change, the listener is notified.
     */
    void refresh() {
        SDState newState = SD.begin(chipSelectPin) ? PRESENT : NOT_PRESENT;

        if (newState != state) {
            listener(newState);
            state = newState;
        }
    }

    /**
     * @brief Set the listener function to be notified of SD card state changes.
     * @param _listener The function to be called when the SD card state changes.
     */
    void setListener(void (*_listener)(SDState)) {
        listener = _listener;
    }

    /**
     * @brief Get the current state of the SD card.
     * @return The current state of the SD card (PRESENT, NOT_PRESENT, UNKNOWN).
     */
    SDState getCardState() {
        return state;
    }

    /**
     * @brief Check if the SD card is present.
     * @return True if the SD card is present, false otherwise.
     */
    bool isCardPresent() {
        return getCardState() == PRESENT;
    }

    /**
     * @brief Check if the SD card is not present.
     * @return True if the SD card is not present, false otherwise.
     */
    bool isCardNotPresent() {
        return getCardState() == NOT_PRESENT;
    }

    /**
     * @brief Check if the state of the SD card is unknown.
     * @return True if the state of the SD card is unknown, false otherwise.
     */
    bool isCardStateUnknown() {
        return getCardState() == UNKNOWN;
    }

    namespace {
        /**
         * @brief The chip select pin for the SD card.
         * 
         * This is the pin that is used as the chip select when interacting with the SD card.
         */
        uint8_t chipSelectPin = 0;

        /**
         * @brief The current state of the SD card.
         * 
         * This variable holds the current state of the SD card (PRESENT, NOT_PRESENT, UNKNOWN).
         */
        SDState state = UNKNOWN;

        /**
         * @brief The listener function to be called on state changes.
         * 
         * This function pointer holds the address of the listener function
         * that will be called when the state of the SD card changes.
         */
        void (*listener)(SDState);
    }
}