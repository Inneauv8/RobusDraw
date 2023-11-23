#include "RobusDraw.h"

namespace RobusDraw {
    bool initialize(int chipSelect) {

        SERVO_Enable(PENCIL_DOWN_SERVO);
        SERVO_Enable(PENCIL_COLOR_SERVO);
        setPencilDown(false);
        return SD.begin(chipSelect);
    }

    void update() {
        
        bool inTimout = timeoutState.inTimeout();
        if (isDrawingLoaded() && isDrawingRunning() && !isDrawingFinished() && !inTimout) {
            RobusPosition::startFollowingTarget();

            RobusPosition::Vector position = RobusPosition::getPosition();
            DrawingPoint point = getLoadedPoint();

            if (dist(point.x, point.y, position.x, position.y) < precision && isDrawingLoaded() && isDrawingRunning() && !isDrawingFinished()) {
                point = loadNextPoint();
                RobusPosition::setTarget(point.x, point.y);
            }
            setPencilDown(state.inLine);
        } else {
            RobusPosition::stopFollowingTarget();
            RobusMovement::stop();
            
            if (inTimout) {
                setPencilDown(timeoutState.pencilDown);
            }
        }

        RobusPosition::update();
    }

    void getDrawingName(char* name) {
        strcpy(info.name, name);
    }

    float getDrawingWidth() {
        return info.width;
    }

    float getDrawingHeight() {
        return info.height;
    }

    int getDrawingSize() {
        return info.pointsCount;
    }

    void setPencilDown(bool enabled) {
        float angle = enabled ? PENCIL_DOWN_ANGLE : PENCIL_UP_ANGLE;
        SERVO_SetAngle(PENCIL_DOWN_SERVO, angle);
    }

    void setPencilColor(PencilColor color) {
        int currentAngle = pencilColorToAngle(state.color);
        int targetAngle = pencilColorToAngle(color);
        if (currentAngle != targetAngle) {
            if (color != NONE) {
                unsigned long distance = fabs(targetAngle - currentAngle) / PENCIL_BETWEEN_ANGLE;
                timeout(distance * PENCIL_CHANGE_TIME, false);
                
                SERVO_SetAngle(PENCIL_COLOR_SERVO, targetAngle);
            }
            state.color = color;
        }
    }

    void setPrecision(float _precision) {
        precision = _precision;
    }

    float getPrecision() {
        return precision;
    }

    bool loadDrawing(char* path) {
        if (!SD.exists(path)) {
            Serial.println("ROBUS DRAW Can't find requested file");
            return false;
        }

        boolean readingInfo = false;
        boolean infoExtracted = false;

        boolean readingSettings = false;
        boolean settingsExtracted = false;

        state = {};
        settings = {};
        state.drawing = false;
        state.inLine = false;
        state.loaded = false;
        state.pointIndex = 0;

        state.drawingFile.close();
        state.drawingFile = SD.open(path);

        if (!state.drawingFile) {
            return false;
        }

        //Serial.println(state.drawingFile.available() ? "true" : "false");
        
        while (state.drawingFile.available() && (!settingsExtracted || !infoExtracted)) {
            char line[50] = "\0";

            getFileNextLine(line, 50);

            // Info deserialization
            if (strcmp(line, INFO_START_TAG) == 0) {
                readingInfo = true;
                infoExtracted = false;
            }

            if (readingInfo) {
                int index = indexOf(line, '=') + 2;

                char substr[50] = "\0";
                substring(line, substr, index);

                if (startsWith("name", line)) {
                    strcpy(info.name, substr);
                } else if (startsWith("width", line)) {
                    info.width = atof(substr);
                } else if (startsWith("height", line)) {
                    info.height = atof(substr);
                } else if (startsWith("pointsCount", line)) {
                    info.pointsCount = atoi(substr);
                }
            }
            
            if (readingInfo && strcmp(line, INFO_END_TAG) == 0) {
                readingInfo = false;
                infoExtracted = true;
            }
            
            // Settings deserialization
            if (strcmp(line, SETTINGS_START_TAG) == 0) {
                readingSettings = true;
                settingsExtracted = false;
            }

            if (readingSettings) {
                int index = indexOf(line, '=') + 2;

                char substr[50] = "\0";
                substring(line, substr, index);

                if (startsWith("followAngularVelocityScale", line)) {
                    settings.followAngularVelocityScale = atof(substr);
                } else if (startsWith("followVelocity", line)) {
                    settings.followVelocity = atof(substr);
                } else if (startsWith("curveTightness", line)) {
                    settings.curveTightness = atof(substr);
                }
            }
            
            if (readingSettings && strcmp(line, SETTINGS_END_TAG) == 0) {
                readingSettings = false;
                settingsExtracted = true;
            }
        }

        bool drawingHeaderFound = false;
        while (state.drawingFile.available()) {
            char line[50];

            getFileNextLine(line, 50);

            if (strcmp(line, DRAWING_START_TAG) == 0) {
                drawingHeaderFound = true;
                break;
            }
        }

        if (!infoExtracted) {
            Serial.println("ROBUS DRAW Missing drawing info");
            return false;
        }

        if (!settingsExtracted) {
            Serial.println("ROBUS DRAW Missing drawing settings");
            return false;
        }

        if (!drawingHeaderFound) {
            Serial.println("ROBUS DRAW Missing drawing points data");
            return false;
        }

        state.loaded = true;

        return true;
    }

    void startDrawing() {
        if (isDrawingLoaded() && !isDrawingFinished() && !isDrawingRunning()) {
            state.drawing = true;
            state.inLine = false;
            state.pointIndex = 0;
            setPencilDown(state.inLine);
        }
    }
    void restartDrawing() {
        if (isDrawingLoaded() && !isDrawingPaused()) {
            char* fileName = state.drawingFile.name();
            state.drawingFile.close();
            loadDrawing(fileName);

            startDrawing();
        }
    }

    void resumeDrawing() {
        if (isDrawingPaused()) {
            state.drawing = true;
        }
    }

    void pauseDrawing() {
        if (!isDrawingPaused() && !isDrawingFinished() && isDrawingLoaded()) {
            state.drawing = false;
        }
    }

    void stopDrawing() {
        state.loaded = false;
        state.inLine = false;
        state.drawing = false;
        state.pointIndex = 0;
        state.drawingFile.close();
    }

    bool isDrawingPaused() {
        return !isDrawingRunning() && !isDrawingFinished() && isDrawingLoaded();
    }

    bool isDrawingRunning() {
        return state.drawing && isDrawingLoaded();
    }

    bool isDrawingFinished() {
        return (state.pointIndex >= info.pointsCount - 1) && isDrawingLoaded();
    }

    bool isDrawingLoaded() {
        return state.loaded;
    }

    float getProgress() {
        return float(state.pointIndex) / float(info.pointsCount - 1.0);
    }

    DrawingInfo getDrawingInfo() {
        return info;
    }
    DrawingSettings getDrawingSettings() {
        return settings;
    }

    namespace {
        DrawingState state = {};
        TimoutState timeoutState = {};
        DrawingInfo info = {};
        DrawingSettings settings = {};
        DrawingPoint loadedPoint = {};

        float precision = 1;

        DrawingPoint getLoadedPoint() {
            return loadedPoint;
        }
        
        DrawingPoint loadNextPoint() {
            if (isDrawingLoaded() && state.pointIndex < info.pointsCount) {
                if (loadedPoint.isBoundary) {
                    state.inLine = !state.inLine;
                }

                char line[100] = "\0";
                getFileNextLine(line, 100);

                char* tokens[8];
                int tokenCount;

                split(line, " ", tokens, &tokenCount);

                if (tokenCount >= 4) {
                    loadedPoint.x = atof(tokens[0]);
                    loadedPoint.y = atof(tokens[1]);

                    loadedPoint.color = stringToPencilColor(tokens[2]);
                    loadedPoint.isBoundary = strcmp(tokens[3], "true") == 0 ? true : false;
                } else {
                    Serial.println("DEBUG DRAW : too many elements in drawing points");
                }
                state.pointIndex++;

                setPencilColor(loadedPoint.color);
            }
            return loadedPoint;
        }

        void timeout(unsigned long time, bool isPencilDown) {
            timeoutState.time = millis() + time;
            timeoutState.pencilDown = isPencilDown;
        }

        void getFileNextLine(char* line, int size) {
            line[size - 1] = '\0';

            for (int i = 0; i < size; i++) {
                char c = state.drawingFile.read();
                if (c == '\n' || state.drawingFile.available() == 0) {
                    line[i] = '\0';
                    return;
                }
                line[i] = c;
            }
        }

        bool startsWith(const char *start, const char *text) {
            if (strlen(start) > strlen(text)) {
                return false;
            }

            for (int i = 0; i < strlen(start); i++) {
                if (start[i] != text[i]) {
                    return false;
                }
            }

            return true;
        }

        int indexOf(const char* text, char target) {
            const char* result = strchr(text, target);
            if (result != nullptr) {
                return result - text; // Calculate the index
            } else {
                return -1; // Character not found
            }
        }

        void substring(char* text, char* substring, unsigned int index) {
            int subIndex = 0;
            for (int i = index; i < strlen(text); i++) {
                substring[subIndex] = text[i];
                subIndex++;
            }
        }

        void split(char *str, char *delim, char **result, int *count) {
            char *token = strtok(str, delim);
            *count = 0;

            while (token != NULL) {
                result[(*count)++] = token;
                token = strtok(NULL, delim);
            }
        }
    }
}