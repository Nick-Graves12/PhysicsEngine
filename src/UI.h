#pragma once
#include "raylib.h"

extern Font uiFont;

void UIText(const char* text, float x, float y, float size, Color color);
void UILabelValue(const char* label, const char* value, float x, float y);
