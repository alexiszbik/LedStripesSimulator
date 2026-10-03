#include "RaylibUi.h"

#include "midi/MidiMapping.h"

#include <raylib.h>

#include <algorithm>
#include <cstdio>

namespace
{
    
constexpr int kDefaultWindowWidth = 800;
constexpr int kDefaultWindowHeight = 340;
constexpr int kTargetFps = 60;
constexpr int kStripeColumns = 3;
constexpr int kStripeRows = 2;
constexpr int kHeaderHeight = 0;
constexpr float kPanelPadding = 8.0f;

constexpr int kCcFontSize = 18;
constexpr int kCcPanelPaddingTop = 4;
constexpr int kCcPanelPaddingBottom = 4;
constexpr int kCcSecondLineOffset = 26;

float controlsPanelHeightPx()
{
    return static_cast<float>(kCcPanelPaddingTop + kCcSecondLineOffset + kCcFontSize + kCcPanelPaddingBottom);
}

int clampChannel(int value)
{
    return std::max(0, std::min(255, value));
}

::Color textColorForBackground(int r, int g, int b)
{
    const int luminance = (r * 299 + g * 587 + b * 114) / 1000;
    return luminance > 128 ? BLACK : WHITE;
}
} // namespace

RaylibUi::RaylibUi() = default;

RaylibUi::~RaylibUi()
{
    shutdown();
}

bool RaylibUi::init()
{
    if (initialized_)
        return true;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(kDefaultWindowWidth, kDefaultWindowHeight, "LedStripes");
    SetTargetFPS(kTargetFps);

    initialized_ = true;
    return IsWindowReady();
}

void RaylibUi::shutdown()
{
    if (!initialized_)
        return;

    CloseWindow();
    initialized_ = false;
}

bool RaylibUi::shouldClose() const
{
    if (!initialized_)
        return true;

    return WindowShouldClose() || quitRequested_;
}

void RaylibUi::pollInput()
{
    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_ESCAPE))
        quitRequested_ = true;
}

void RaylibUi::computeLayout(int screenWidth,
                             int screenHeight,
                             Rect stripesOut[6],
                             Rect& controlsOut) const
{
    const float controlsHeight = controlsPanelHeightPx();
    const float stripesAreaHeight =
        std::max(120.0f, static_cast<float>(screenHeight) - controlsHeight - kHeaderHeight);

    controlsOut.x = 0.0f;
    controlsOut.y = static_cast<float>(screenHeight) - controlsHeight;
    controlsOut.width = static_cast<float>(screenWidth);
    controlsOut.height = controlsHeight;

    const float cellWidth = static_cast<float>(screenWidth) / static_cast<float>(kStripeColumns);
    const float cellHeight = stripesAreaHeight / static_cast<float>(kStripeRows);

    for (int index = 0; index < MidiMapping::kStripeCount; ++index)
    {
        const int row = index / kStripeColumns;
        const int col = index % kStripeColumns;

        Rect& rect = stripesOut[index];
        rect.x = static_cast<float>(col) * cellWidth + kPanelPadding;
        rect.y = static_cast<float>(kHeaderHeight) + static_cast<float>(row) * cellHeight + kPanelPadding;
        rect.width = cellWidth - (2.0f * kPanelPadding);
        rect.height = cellHeight - (2.0f * kPanelPadding);
        rect.width = std::max(rect.width, 32.0f);
        rect.height = std::max(rect.height, 32.0f);
    }
}

void RaylibUi::drawStripe(std::size_t index, const Rect& rect, int r, int g, int b) const
{
    r = clampChannel(r);
    g = clampChannel(g);
    b = clampChannel(b);

    const ::Color fill = {
                         static_cast<unsigned char>(r),
                         static_cast<unsigned char>(g),
                         static_cast<unsigned char>(b),
                         255 };

    DrawRectangle(static_cast<int>(rect.x),
                  static_cast<int>(rect.y),
                  static_cast<int>(rect.width),
                  static_cast<int>(rect.height),
                  fill);

    DrawRectangleLines(static_cast<int>(rect.x),
                       static_cast<int>(rect.y),
                       static_cast<int>(rect.width),
                       static_cast<int>(rect.height),
                       Fade(BLACK, 0.35f));
}

const char* RaylibUi::modeLabel(StripeMode mode, bool ccMode)
{
    if (ccMode)
        return "CC level mode";

    switch (mode)
    {
    case StripeMode::Rainbow:
        return "Rainbow";
    case StripeMode::Explode:
        return "Explode";
    case StripeMode::Normal:
    default:
        return "Normal";
    }
}

void RaylibUi::drawControlsPanel(const Rect& rect, const ControlParameters& params) const
{
    DrawRectangle(static_cast<int>(rect.x),
                  static_cast<int>(rect.y),
                  static_cast<int>(rect.width),
                  static_cast<int>(rect.height),
                  { 22, 22, 26, 255 });

    DrawLine(static_cast<int>(rect.x),
             static_cast<int>(rect.y),
             static_cast<int>(rect.x + rect.width),
             static_cast<int>(rect.y),
             Fade(RAYWHITE, 0.25f));

    char line1[256];
    std::snprintf(line1,
                  sizeof(line1),
                  "CC%2d Decay: %3d    | CC%2d Rainbow speed: %3d | CC%2d Explode level: %3d",
                  MidiMapping::kCcDecay,
                  params.decayCc,
                  MidiMapping::kCcRainbowSpeed,
                  params.rainbowSpeedCc,
                  MidiMapping::kCcExplodeLevel,
                  params.explodeLevelCc);

    char line2[256];
    std::snprintf(line2,
                  sizeof(line2),
                  "CC%2d CC mode: %3d | CC%2d Level: %3d",
                  MidiMapping::kCcMode,
                  params.modeCc,
                  MidiMapping::kCcLevel,
                  params.levelCc);

    const int textY = static_cast<int>(rect.y) + kCcPanelPaddingTop;
    DrawText(line1, 16, textY, kCcFontSize, RAYWHITE);
    DrawText(line2, 16, textY + kCcSecondLineOffset, kCcFontSize, RAYWHITE);
}

void RaylibUi::draw(const StripeEngine& engine, const std::string& portName) const
{
    if (!initialized_) {
        return;
    }

    BeginDrawing();
    ClearBackground({ 0, 0, 0, 255 });

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    Rect stripes[6]{};
    Rect controlsRect{};
    computeLayout(screenWidth, screenHeight, stripes, controlsRect);

    for (int stripe = 0; stripe < MidiMapping::kStripeCount; ++stripe)
    {
        int r = 0;
        int g = 0;
        int b = 0;
        engine.getStripeRgb(static_cast<std::size_t>(stripe), r, g, b);
        drawStripe(static_cast<std::size_t>(stripe), stripes[stripe], r, g, b);
    }

    drawControlsPanel(controlsRect, engine.controlParameters());
    EndDrawing();
}
