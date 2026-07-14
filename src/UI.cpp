#include "UI.h"

Font uiFont;

void UIText(const char* text, float x, float y, float size, Color color)
{
    DrawTextEx(uiFont, text, { x, y }, size, 1.0f, color);
}

void UILabelValue(const char* label, const char* value, float x, float y)
{
    UIText(label, x, y, 16, DARKBLUE);
    UIText(value, x + 60, y, 16, BLACK);
}

