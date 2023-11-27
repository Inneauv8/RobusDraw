#include <Arduino.h>
#include <LibRobus.h>
#include <SDState.h>

#define INTEGRATION_ITERATION 50
#include "RobusDraw.h"
#include <BluetoothDraw.h>

#define DEFAULT 0
#define LABYRINTHE 1
#define DEFAULTDRAWING 2
#define SDDRAWING 3

#define SUCESS_TONE 1500
#define SUCESS_TONE_DURATION 350

#define FAILURE_TONE 550
#define FAILURE_TONE_DURATION 300

#define CHANING_MENU_TONE 300
#define CHANING_MENU_TONE_DURATION 125

void onSDStateChange(SDState::SDState state);
void updateButtonState();
bool isButtonReleased(int button);
void resultFeedback(bool success);
void changeMenuFeedback();

bool lastButtonState[4] = {0};
bool buttonState[4] = {0};
int state = DEFAULT;

void setup()
{
    BoardInit();

    Serial.begin(9600);
    Serial3.begin(115200);
    BluetoothDraw::initialize(Serial3);

    RobusDraw::setPrecision(0.4);
    RobusPosition::setCurveTightness(50);
    RobusPosition::setFollowAngularVelocityScale(3); //3
    RobusMovement::setPIDAngular(0.5, 0, 0.01, 0);

    RobusDraw::initialize();

    SDState::setListener(onSDStateChange);
    SDState::registerCard(10);
}

void onSDStateChange(SDState::SDState state) {
    resultFeedback(state == SDState::PRESENT);

    if (state == SDState::PRESENT) {
        Serial.println("Successfully initialized SD card!");
    } else {
        Serial.println("Failed to initialize SD card!");
    }
}

bool loadWithFeedback(char *path) {
    bool success = RobusDraw::loadDrawing(path);

    resultFeedback(success);

    return success;
}

void resultFeedback(bool success) {
    AX_BuzzerON(success ? SUCESS_TONE : FAILURE_TONE, success ? SUCESS_TONE_DURATION : FAILURE_TONE_DURATION);
}

void changeMenuFeedback() {
    AX_BuzzerON(CHANING_MENU_TONE, CHANING_MENU_TONE_DURATION);
}

void loop()
{   
    SDState::refresh();
    
    if (SDState::isCardPresent()) {
        BluetoothDraw::ReadingState bluetoothState = BluetoothDraw::update();
        if (bluetoothState == BluetoothDraw::ReadingState::DONE || bluetoothState == BluetoothDraw::ReadingState::FAILED) {
            resultFeedback(bluetoothState == BluetoothDraw::ReadingState::DONE);
        }
    }

    updateButtonState();
    delay(2);

    switch (state)
    {
    case LABYRINTHE:

        if (isButtonReleased(LEFT))
        {
            Serial.println("Facile");
            state = DEFAULT;
        }

        if (isButtonReleased(FRONT))
        {
            Serial.println("Moyen");
            state = DEFAULT;
        }

        if (isButtonReleased(RIGHT))
        {
            Serial.println("Difficile");
            state = DEFAULT;
        }
        break;

    case DEFAULTDRAWING:
        if (isButtonReleased(LEFT))
        {
           if (loadWithFeedback("SD1.TXT")) {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(FRONT))
        {
            RobusDraw::stopDrawing();
            if (loadWithFeedback("SD2.TXT")) {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(RIGHT))
        {
            if (loadWithFeedback("SD3.TXT")) {
                state = DEFAULT;
            }
        }
        break;
    case SDDRAWING:
        if (isButtonReleased(LEFT))
        {
            if (loadWithFeedback("SPECIAL1.TXT")) {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(FRONT))
        {
            if (loadWithFeedback("SPECIAL2.TXT")) {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(RIGHT))
        {
            if (loadWithFeedback("SPECIAL3.TXT")) {
                state = DEFAULT;
            }
        }
        break;
    default:
        if (isButtonReleased(LEFT))
        {
            Serial.println("Labyrinthe");
            state = LABYRINTHE;
            changeMenuFeedback();
        }

        if (isButtonReleased(FRONT))
        {
            Serial.println("Default Drawing");
            state = DEFAULTDRAWING;
            changeMenuFeedback();
        }

        if (isButtonReleased(RIGHT))
        {
            Serial.println("SD Drawing");
            state = SDDRAWING;
            changeMenuFeedback();
        }

        if (isButtonReleased(REAR))
        {
            Serial.println("Reset");
            changeMenuFeedback();
        }
        break;
    }

    if (RobusDraw::isDrawingLoaded() && !RobusDraw::isDrawingRunning()) {
        RobusDraw::startDrawing();
    }
    RobusDraw::update();
}

void updateButtonState()
{
    for (int i = 0; i < 4; i++)
    {
        lastButtonState[i] = buttonState[i];
        buttonState[i] = ROBUS_IsBumper(i);
    }
}

bool isButtonReleased(int button)
{
    return !buttonState[button] && lastButtonState[button];
}