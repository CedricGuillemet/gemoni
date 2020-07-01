#pragma once
#include <stdint.h>

static const float PI = 3.141592f;
inline float RadToDeg(float a)
{
    return a * 180.f / PI;
}
inline float DegToRad(float a)
{
    return a / 180.f * PI;
}

template<typename T>
void Swap(T& a, T& b)
{
    T temp = a;
    a = b;
    b = temp;
}

template<typename T>
T Min(const T& a, const T& b)
{
    return (a < b) ? a : b;
}

template<typename T>
T Max(const T& a, const T& b)
{
    return (a > b) ? a : b;
}

template<typename T>
T Lerp(T a, T b, float t)
{
    return T(a + (b - a) * t);
}

inline float Saturate(float v)
{
    return Min(Max(v, 0.f), 1.f);
}

inline uint32_t ColorU8(uint8_t R, uint8_t G, uint8_t B, uint8_t A)
{
    return (((uint32_t)(A) << 24) | ((uint32_t)(B) << 16) | ((uint32_t)(G) << 8) | ((uint32_t)(R)));
}

inline uint32_t ColorF32(float R, float G, float B, float A)
{
    uint32_t out;
    out = (uint32_t)(Saturate(R) * 255.f + 0.5f) << 0;
    out |= (uint32_t)(Saturate(G) * 255.f + 0.5f) << 8;
    out |= (uint32_t)(Saturate(B) * 255.f + 0.5f) << 16;
    out |= (uint32_t)(Saturate(A) * 255.f + 0.5f) << 24;
    return out;
}

typedef void (*LogOutput)(const char* szText);
void AddLogOutput(LogOutput output);
int Log(const char* szFormat, ...);