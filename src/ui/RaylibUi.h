#pragma once

#include "midi/StripeEngine.h"

#include <string>

class RaylibUi
{
public:
    RaylibUi();
    ~RaylibUi();

    RaylibUi(const RaylibUi&) = delete;
    RaylibUi& operator=(const RaylibUi&) = delete;

    bool init();
    void shutdown();

    bool shouldClose() const;
    void pollInput();
    void draw(const StripeEngine& engine, const std::string& portName) const;

private:
    struct Rect
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    void computeLayout(int screenWidth,
                       int screenHeight,
                       Rect stripesOut[6],
                       Rect& controlsOut) const;

    void drawStripe(std::size_t index, const Rect& rect, int r, int g, int b) const;
    void drawControlsPanel(const Rect& rect, const ControlParameters& params) const;

    static const char* modeLabel(StripeMode mode, bool ccMode);

    bool initialized_ = false;
    bool quitRequested_ = false;
};
