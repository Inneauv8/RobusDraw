#ifndef ROBUS_DRAW_H
#define ROBUS_DRAW_H

#include <LibRobus.h>
#include <RobusPosition.h>
#include <PencilColor.h>
#include <SPI.h>
#include <SD.h>

#define PENCIL_UP_ANGLE 83
#define PENCIL_DOWN_ANGLE 100 //95

#define PENCIL_DOWN_SERVO SERVO_1


namespace RobusDraw {
    
    struct DrawingInfo {
        char name[20];
        float width;
        float height;
        int pointsCount;
    };

    struct DrawingSettings {
        float followAngularVelocityScale = NAN; /**< Scale factor for angular velocity when following a target. */
        float followVelocity = NAN; /**< Velocity at which the robot follows a target. */
        float curveTightness = NAN; /**< Tightness of the curve when following a target. */
    };

    struct DrawingPoint {
        float x;
        float y;
        PencilColor color;
        bool isBoundary;
    };

    struct DrawingState {
        bool loaded = false;
        int pointIndex = 0;
        bool inLine = false;

        bool drawing = false;

        File drawingFile;
    };

    void initialize(int chipSelect);
    void update();

    void getDrawingName(char* name);
    float getDrawingWidth();
    float getDrawingHeight();
    int getDrawingSize();


    void setPencilDown(bool enabled);

    void setPrecision(float _precision);
    float getPrecision();

    bool loadDrawing(char* path);
    void startDrawing();
    void restartDrawing();
    void resumeDrawing();
    void pauseDrawing();
    void stopDrawing();

    bool isDrawingPaused();
    bool isDrawingRunning();
    bool isDrawingFinished();
    bool isDrawingLoaded();

    float getProgress();

    DrawingInfo getDrawingInfo();
    DrawingSettings getDrawingSettings();

    namespace {
        static const char INFO_START_TAG[] = "DRAWING_INFO_START";
        static const char INFO_END_TAG[]= "DRAWING_INFO_END";
        static const char SETTINGS_START_TAG[] = "SETTINGS_START";
        static const char SETTINGS_END_TAG[] = "SETTINGS_END";
        static const char DRAWING_START_TAG[] = "DRAWING_START";
        static const char DRAWING_END_TAG[] = "DRAWING_END";
        static const int SINGLE_PENCIL_SWITCH_TIME = 20;

        extern DrawingState state;
        extern DrawingInfo info;
        extern DrawingSettings settings;
        extern DrawingPoint loadedPoint;
        extern float precision;

        DrawingPoint getLoadedPoint();
        DrawingPoint loadNextPoint();

        void getFileNextLine(char* line, int size);
        bool startsWith(const char *start, const char *text);
        int indexOf(const char* text, char target);
        void substring(char* text, char* substring, unsigned int index);
        void split(char *str, char *delim, char **result, int *count);
    }
}

#endif