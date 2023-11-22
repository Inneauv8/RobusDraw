#ifndef PENCILCOLOR_H
#define PENCILCOLOR_H

#include <Arduino.h>  // Include for strcmp function

enum PencilColor {
    RED,
    BLUE,
    GREEN,
    BLACK,
    NONE
};

const char* pencilColorToString(PencilColor color);

PencilColor stringToPencilColor(const char* str);

int pencilColorToAngle(PencilColor color);

#endif // PENCILCOLOR_H
