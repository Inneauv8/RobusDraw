/**
 * @file RobusDraw.h
 * @brief Provides functionality for controlling a drawing robot equipped with servo-controlled pencils.
 */


#include "RobusDraw.h"

/**
 * @namespace RobusDraw
 * @brief Namespace encapsulating functionality for controlling a drawing robot.
 */
namespace RobusDraw {
    /**
     * @brief Initializes the drawing system by enabling the necessary servos and setting the initial pencil state.
     */
    void initialize() {
        SERVO_Enable(PENCIL_DOWN_SERVO);
        SERVO_Enable(PENCIL_COLOR_SERVO);
        setPencilDown(false);
    }

    /**
     * @brief Manages the main update loop, controlling the movement of the drawing robot based on loaded drawing data.
     */
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

    /**
     * @brief Retrieves the name of the loaded drawing.
     * @param name A pointer to a character array to store the drawing name.
     */
    void getDrawingName(char* name) {
        strcpy(info.name, name);
    }

    /**
     * @brief Retrieves the width of the loaded drawing.
     * @return The width of the loaded drawing.
     */

    float getDrawingWidth() {
        return info.width;
    }

    /**
     * @brief Retrieves the height of the loaded drawing.
     * @return The height of the loaded drawing.
     */
    float getDrawingHeight() {
        return info.height;
    }

    /**
     * @brief Retrieves the total number of points in the loaded drawing.
     * @return The total number of points in the loaded drawing.
     */
    int getDrawingSize() {
        return info.pointsCount;
    }
    
    /**
     * @brief Lowers or raises the pencil based on the specified state.
     * @param enabled True to lower the pencil, false to raise it.
     */
    void setPencilDown(bool enabled) {
        float angle = enabled ? PENCIL_DOWN_ANGLE : PENCIL_UP_ANGLE;
        SERVO_SetAngle(PENCIL_DOWN_SERVO, angle);
    }

    /**
     * @brief Sets the color of the pencil based on the provided enumeration.
     * @param color The color to set for the pencil.
     */
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

    /**
     * @brief Sets the precision for determining when to move to the next point in the drawing.
     * @param _precision The precision value to set.
     */
    void setPrecision(float _precision) {
        precision = _precision;
    }

    /**
     * @brief Retrieves the current precision value.
     * @return The current precision value.
     */
    float getPrecision() {
        return precision;
    }

    /**
     * @brief Loads a drawing from the specified file path, extracting information and settings.
     * @param path The file path of the drawing to load.
     * @return True if the drawing is loaded successfully, false otherwise.
     */
    bool loadDrawing(char* path) {
        if (!SDState::isCardPresent() || !SD.exists(path)) {
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
        
        /**
         * @brief Initiates the drawing process if a drawing is loaded and not finished or running.
         */
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

    /**
     * @brief Restarts the drawing from the beginning, useful for resetting or replaying a drawing.
     */
    void startDrawing() {
        if (isDrawingLoaded() && !isDrawingFinished() && !isDrawingRunning()) {
            state.drawing = true;
            state.inLine = false;
            state.pointIndex = 0;
            setPencilDown(state.inLine);
        }
    }

    /**
     * @brief Resumes a paused drawing.
     */
    void restartDrawing() {
        if (isDrawingLoaded() && !isDrawingPaused()) {
            char* fileName = state.drawingFile.name();
            state.drawingFile.close();
            loadDrawing(fileName);

            startDrawing();
        }
    }

    /**
     * @brief Pauses the current drawing.
     */
    void resumeDrawing() {
        if (isDrawingPaused()) {
            state.drawing = true;
        }
    }

    /**
     * @brief Stops the current drawing, closing the drawing file.
     */
    void pauseDrawing() {
        if (!isDrawingPaused() && !isDrawingFinished() && isDrawingLoaded()) {
            state.drawing = false;
        }
    }

    /**
     * @brief Checks if the drawing is currently paused.
     * @return True if the drawing is paused, false otherwise.
     */
    void stopDrawing() {
        state.loaded = false;
        state.inLine = false;
        state.drawing = false;
        state.pointIndex = 0;
        state.drawingFile.close();
    }

    /**
     * @brief Checks if the drawing is currently in progress.
     * @return True if the drawing is running, false otherwise.
     */
    bool isDrawingPaused() {
        return !isDrawingRunning() && !isDrawingFinished() && isDrawingLoaded();
    }

    /**
     * @brief Checks if the drawing has been completed.
     * @return True if the drawing is finished, false otherwise.
     */
    bool isDrawingRunning() {
        return state.drawing && isDrawingLoaded();
    }

    /**
     * @brief Checks if a drawing is loaded and the SD card is present.
     * @return True if a drawing is loaded and the SD card is present, false otherwise.
     */
    bool isDrawingFinished() {
        return (state.pointIndex >= info.pointsCount - 1) && isDrawingLoaded();
    }

    /**
     * @brief Retrieves the progress of the current drawing as a percentage.
     * @return The progress of the drawing as a percentage.
     */
    bool isDrawingLoaded() {
        return state.loaded && SDState::isCardPresent();
    }

    /**
     * @brief Retrieves the information (name, width, height, point count) of the loaded drawing.
     * @return The information of the loaded drawing.
     */
    float getProgress() {
        return float(state.pointIndex) / float(info.pointsCount - 1.0);
    }

    /**
     * @brief Retrieves the settings (angular velocity scale, velocity, curve tightness) of the loaded drawing.
     * @return The settings of the loaded drawing.
     */
    DrawingInfo getDrawingInfo() {
        return info;
    }
    DrawingSettings getDrawingSettings() {
        return settings;
    }

    namespace {
        /**
         * @brief Represents the state of the drawing system.
         */
        DrawingState state = {};
        /**
         * @brief Represents the state of the timeout mechanism.
         */
        TimoutState timeoutState = {};
        /**
         * @brief Represents information about the loaded drawing.
         */
        DrawingInfo info = {};
        /**
         * @brief Represents settings for the drawing system.
         */
        DrawingSettings settings = {};
        /**
         * @brief Represents the current loaded drawing point.
         */
        DrawingPoint loadedPoint = {};

        /**
         * @brief Represents the precision for determining when to move to the next point in the drawing.
         */
        float precision = 1;

        /**
         * @brief Retrieves the currently loaded drawing point.
         * @return The currently loaded drawing point.
         */
        DrawingPoint getLoadedPoint() {
            return loadedPoint;
        }
        
        /**
         * @brief Loads the next point in the drawing sequence.
         * @return The next point in the drawing sequence.
         */
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

        /**
         * @brief Sets a timeout for a specified duration with an optional pencil state.
         * @param time The duration of the timeout in milliseconds.
         * @param isPencilDown The state of the pencil during the timeout.
         */
        void timeout(unsigned long time, bool isPencilDown) {
            timeoutState.time = millis() + time;
            timeoutState.pencilDown = isPencilDown;
        }

        /**
         * @brief Reads the next line from the drawing file.
         * @param line A character array to store the read line.
         * @param size The size of the character array.
         */
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

        /**
         * @brief Checks if a string starts with a specified prefix.
         * @param start The prefix to check.
         * @param text The text to check for the prefix.
         * @return True if the text starts with the specified prefix, false otherwise.
         */
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

        /**
         * @brief Finds the index of a specified character in a string.
         * @param text The text to search.
         * @param target The character to find.
         * @return The index of the character in the string, or -1 if not found.
         */
        int indexOf(const char* text, char target) {
            const char* result = strchr(text, target);
            if (result != nullptr) {
                return result - text; // Calculate the index
            } else {
                return -1; // Character not found
            }
        }

        /**
         * @brief Extracts a substring from a given text starting from a specified index.
         * @param text The text to extract from.
         * @param substring A character array to store the extracted substring.
         * @param index The starting index of the substring.
         */
        void substring(char* text, char* substring, unsigned int index) {
            int subIndex = 0;
            for (int i = index; i < strlen(text); i++) {
                substring[subIndex] = text[i];
                subIndex++;
            }
        }

        /**
         * @brief Splits a string into tokens using a specified delimiter.
         * @param str The string to split.
         * @param delim The delimiter used for splitting.
         * @param result An array to store the resulting tokens.
         * @param count A pointer to store the number of tokens.
         */
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