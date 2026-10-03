#pragma once

#include "LedColor.h"
#include "ControlParameters.h"
#include "LEDStatus.h"
#include "MidiMapping.h"

#include <array>
#include <cstddef>

enum class StripeMode
{
    Normal,
    Rainbow,
    Explode
};

class StripeEngine
{
public:
    StripeEngine();

    void setup();
    void tick(unsigned long nowMs);

    void setNote(unsigned char note, unsigned char velocity);
    void setDecayTime(unsigned char byteValue);
    void setRainbowTime(unsigned char byteValue);
    void setExplodeLevel(unsigned char byteValue);
    void setModeCc(unsigned char byteValue);
    void setCcLevel(unsigned char inCcLevel);

    StripeMode mode() const { return mode_; }
    bool isCcMode() const { return isCcMode_; }

    ControlParameters controlParameters() const;
    void getStripeRgb(std::size_t stripeIndex, int& r, int& g, int& b) const;

private:
    static unsigned char velocityToLevel(unsigned char velocity);
    static unsigned char velocityToLevelQuadratic(unsigned char velocity);

    static constexpr int kColorCount = static_cast<int>(LedColor::Count);
    static constexpr int kLedCount = kColorCount * MidiMapping::kStripeCount;

    int hue_ = 0;
    unsigned long timePassedMs_ = 0;
    unsigned long decayTimeMs_ = 1;
    unsigned long rainbowTimestampMs_ = 0;
    unsigned long rainbowIntervalMs_ = 1;
    StripeMode mode_ = StripeMode::Normal;
    bool needsToClearColors_ = false;
    unsigned long explodeTimestampMs_ = 0;

    std::array<LEDStatus, kLedCount> colorStatus_{};
    bool isCcMode_ = false;
    unsigned char ccLevel_ = 0;

    unsigned char decayCc_ = 0;
    unsigned char rainbowSpeedCc_ = 0;
    unsigned char explodeLevelCc_ = 0;
    unsigned char modeCc_ = 0;
    unsigned char levelCc_ = 0;
};
