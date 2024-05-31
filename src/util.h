#pragma once

int clamp(int d, int min, int max);

float map(float x, float in_min, float in_max, float out_min, float out_max);

float convert_to_soil_wetness(int value);
