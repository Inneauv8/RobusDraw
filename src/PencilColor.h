#ifndef PENCILCOLOR_H
#define PENCILCOLOR_H

#include <Arduino.h>  // Include for strcmp function

enum PencilColor {
    BLACK,
    RED,
    GREEN,
    BLUE,
    NONE
};

const char* pencilColorToString(PencilColor color);

PencilColor stringToPencilColor(const char* str);

#endif // PENCILCOLOR_H
