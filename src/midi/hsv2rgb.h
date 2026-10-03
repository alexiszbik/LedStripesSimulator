#pragma once

struct Rgb
{
    float r;
    float g;
    float b;
};

struct Hsv
{
    float h;
    float s;
    float v;
};

inline Rgb hsvToRgb(Hsv in)
{
    Rgb out{};

    if (in.s <= 0.0f)
    {
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }

    float hh = in.h;
    if (hh >= 360.0f)
        hh = 0.0f;

    hh /= 60.0f;
    const auto i = static_cast<long>(hh);
    const float ff = hh - static_cast<float>(i);
    const float p = in.v * (1.0f - in.s);
    const float q = in.v * (1.0f - (in.s * ff));
    const float t = in.v * (1.0f - (in.s * (1.0f - ff)));

    switch (i)
    {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;
    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }

    return out;
}
