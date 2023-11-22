#include "pencilcolor.h"

const char* pencilColorToString(PencilColor color) {
    switch (color) {
        case PencilColor::BLACK:
            return "BLACK";
        case PencilColor::RED:
            return "RED";
        case PencilColor::GREEN:
            return "GREEN";
        case PencilColor::BLUE:
            return "BLUE";
        case PencilColor::NONE:
            return "NONE";
        default:
            return "UNKNOWN";
    }
}

PencilColor stringToPencilColor(const char* str) {
    if (strcmp(str, "BLACK") == 0) {
        return PencilColor::BLACK;
    } else if (strcmp(str, "RED") == 0) {
        return PencilColor::RED;
    } else if (strcmp(str, "GREEN") == 0) {
        return PencilColor::GREEN;
    } else if (strcmp(str, "BLUE") == 0) {
        return PencilColor::BLUE;
    } else if (strcmp(str, "NONE") == 0) {
        return PencilColor::NONE;
    } else {
        // Default to NONE if the input string doesn't match any known color
        return PencilColor::NONE;
    }
}

int pencilColorToAngle(PencilColor color) {
    switch (color) {
        case PencilColor::BLACK:
            return 90;
        case PencilColor::RED:
            return 50;
        case PencilColor::GREEN:
            return 130;
        case PencilColor::BLUE:
            return 90;
        case PencilColor::NONE:
            return 0;
        default:
            return 0;
    }
}