#include <Arduino.h>
#include <LibRobus.h>
#include <SDState.h>

#define INTEGRATION_ITERATION 50
#include "RobusDraw.h"
#include <BluetoothDraw.h>
#include <music.h>

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

#define PACMAN_CODE_PIN 11

#define LABYRINTH_COUNT 3

void onSDStateChange(SDState::SDState state);
void updateButtonState();
bool isButtonReleased(int button);
void resultFeedback(bool success);
void changeMenuFeedback();
bool loadWithFeedback(char *path);

bool lastButtonState[4] = {0};
bool buttonState[4] = {0};
int state = DEFAULT;

char pacmanFile[50];

Note note4[] = {Note(493,125),{987,125},{740,125},{622,125},{987,63},{698,176},{622,250},{523,125},{1046,146},{784,125},{659,125},{1046,63},{784,167},{659,250}};

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

    //PACMAN pin
    pinMode(PACMAN_CODE_PIN, INPUT_PULLUP);

    SDState::setListener(onSDStateChange);
    SDState::registerCard(10);

    //Init random
    randomSeed(analogRead(A0));
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

bool loadLabyrinth(char path[30], int index) {
    char buf[50];
    strcpy(buf,path);

    SD.remove("pac.txt");
    File file = SD.open("pac.txt", FILE_WRITE);

    char number[10];
    itoa(index, number, 10);
    strcat(buf, number);

    file.print(buf);
    file.print("S.txt");
    file.close();

    char end[5] = ".txt";

    strcat(buf, end);

    return loadWithFeedback(buf);
}

bool loadLabyrinth(char path[30]) {
    return loadLabyrinth(path, random(1, LABYRINTH_COUNT + 1));
}

void resultFeedback(bool success) {
    AX_BuzzerON(success ? SUCESS_TONE : FAILURE_TONE, success ? SUCESS_TONE_DURATION : FAILURE_TONE_DURATION);
}

void changeMenuFeedback() {
    AX_BuzzerON(CHANING_MENU_TONE, CHANING_MENU_TONE_DURATION);
}

void loopPanthom() {

}

void loopPacman() {

}

void loop()
{   
    SDState::refresh();
    
    if (SDState::isCardPresent()) {
        BluetoothDraw::ReadingState bluetoothState = BluetoothDraw::update();
        if (bluetoothState == BluetoothDraw::ReadingState::DONE) {
            setSong(note4, 14, false);
            start();
        } else if (bluetoothState == BluetoothDraw::ReadingState::FAILED) {
             AX_BuzzerON(FAILURE_TONE, FAILURE_TONE_DURATION);
        }
    }

    updateButtonState();
    delay(2);

    switch (state)
    {
    case LABYRINTHE:

        if (isButtonReleased(LEFT))
        {
            if (loadLabyrinth("LabF"))
            {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(FRONT))
        {
            if (loadLabyrinth("LabM"))
            {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(RIGHT))
        {
            if (loadLabyrinth("LabD"))
            {
                state = DEFAULT;
            }
        }
        break;

    case DEFAULTDRAWING:
        if (isButtonReleased(LEFT))
        {
            if (loadWithFeedback("SD1.TXT"))
            {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(FRONT))
        {
            RobusDraw::stopDrawing();
            if (loadWithFeedback("SD2.TXT"))
            {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(RIGHT))
        {
            if (loadWithFeedback("SD3.TXT"))
            {
                state = DEFAULT;
            }
        }
        break;
    case SDDRAWING:
        if (isButtonReleased(LEFT))
        {
            if (loadWithFeedback("SPECIAL1.TXT"))
            {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(FRONT))
        {
            if (loadWithFeedback("SPECIAL2.TXT"))
            {
                state = DEFAULT;
            }
        }

        if (isButtonReleased(RIGHT))
        {
            if (loadWithFeedback("SPECIAL3.TXT"))
            {
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

    if (RobusDraw::isDrawingLoaded() && !RobusDraw::isDrawingRunning())
    {
        RobusDraw::startDrawing();
    }

    RobusDraw::update();
    play();
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