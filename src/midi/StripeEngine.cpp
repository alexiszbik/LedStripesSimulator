#include "StripeEngine.h"

#include "hsv2rgb.h"

#include <cmath>
#include <cstdlib>

namespace
{
constexpr int kRedIndex = static_cast<int>(LedColor::Red);
constexpr int kGreenIndex = static_cast<int>(LedColor::Green);
constexpr int kBlueIndex = static_cast<int>(LedColor::Blue);
constexpr int kColorsPerStripe = static_cast<int>(LedColor::Count);
} // namespace

StripeEngine::StripeEngine() = default;

void StripeEngine::setup()
{
    const unsigned char noteStart = static_cast<unsigned char>(MidiMapping::kNoteStart);
    const unsigned char whiteAll = static_cast<unsigned char>(MidiMapping::kRedNoteAll + kColorsPerStripe);

    for (int stripe = 0; stripe < MidiMapping::kStripeCount; ++stripe)
    {
        const unsigned char noteOffset = static_cast<unsigned char>(stripe * (kColorsPerStripe + 1));
        const unsigned char whiteNote = noteStart + noteOffset + kColorsPerStripe;

        for (int color = 0; color < kColorsPerStripe; ++color)
        {
            const unsigned char note = noteStart + noteOffset + static_cast<unsigned char>(color);
            const unsigned char noteAll = static_cast<unsigned char>(MidiMapping::kRedNoteAll + color);
            const int channelIndex = kColorsPerStripe * stripe + color;

            colorStatus_[static_cast<std::size_t>(channelIndex)].init(
                note, noteAll, whiteNote, whiteAll);
        }
    }
}

void StripeEngine::tick(unsigned long nowMs)
{
    timePassedMs_ = nowMs;

    if ((nowMs - rainbowTimestampMs_) > rainbowIntervalMs_)
    {
        rainbowTimestampMs_ = nowMs;
        hue_ = (hue_ + 2) % 360;
    }

    if (needsToClearColors_)
    {
        for (auto& status : colorStatus_)
            status.clear();

        needsToClearColors_ = false;
    }

    if (isCcMode_)
    {
        for (auto& status : colorStatus_)
            status.update(true, ccLevel_, timePassedMs_);
        return;
    }

    switch (mode_)
    {
    case StripeMode::Normal:
        for (auto& status : colorStatus_)
            status.decayAndApply(timePassedMs_, decayTimeMs_);
        break;

    case StripeMode::Rainbow:
    {
        const Rgb rgbColor = hsvToRgb({ static_cast<float>(hue_), 1.0f, 1.0f });
        for (int stripe = 0; stripe < MidiMapping::kStripeCount; ++stripe)
        {
            const int ledOffset = kColorsPerStripe * stripe;
            colorStatus_[static_cast<std::size_t>(ledOffset + kRedIndex)].level =
                static_cast<int>(rgbColor.r * 255.0f);
            colorStatus_[static_cast<std::size_t>(ledOffset + kGreenIndex)].level =
                static_cast<int>(rgbColor.g * 255.0f);
            colorStatus_[static_cast<std::size_t>(ledOffset + kBlueIndex)].level =
                static_cast<int>(rgbColor.b * 255.0f);
        }

        for (auto& status : colorStatus_)
            status.applyLevel();
        break;
    }

    case StripeMode::Explode:
        if (timePassedMs_ - explodeTimestampMs_ > 40)
        {
            explodeTimestampMs_ = timePassedMs_;

            const int chosenStripe = std::rand() % MidiMapping::kStripeCount;
            const int level = 40 + (std::rand() % (255 - 40 + 1));

            for (int channel = 0; channel < kLedCount; ++channel)
            {
                const int stripeIndex = channel / kColorsPerStripe;
                if (chosenStripe == stripeIndex)
                    colorStatus_[static_cast<std::size_t>(channel)].update(true, level, timePassedMs_);
                else
                    colorStatus_[static_cast<std::size_t>(channel)].update(false, 0, timePassedMs_);
            }
        }

        for (auto& status : colorStatus_)
            status.decayAndApply(timePassedMs_, decayTimeMs_);
        break;
    }
}

void StripeEngine::setNote(unsigned char note, unsigned char velocity)
{
    if (isCcMode_)
        return;

    const unsigned char level = velocityToLevel(velocity);
    const bool isOn = level >= 1;

    switch (note)
    {
    case MidiMapping::kRainbowNote:
        mode_ = isOn ? StripeMode::Rainbow : StripeMode::Normal;
        if (!isOn)
            needsToClearColors_ = true;
        break;

    case MidiMapping::kExplodeNote:
        mode_ = isOn ? StripeMode::Explode : StripeMode::Normal;
        if (!isOn)
            needsToClearColors_ = true;
        break;

    default:
        mode_ = StripeMode::Normal;

        for (auto& status : colorStatus_)
        {
            if (status.note == note || status.noteAll == note || status.noteWhite == note
                || status.noteWhiteAll == note)
            {
                status.update(isOn, level, timePassedMs_);
            }
        }
        break;
    }
}

void StripeEngine::setDecayTime(unsigned char byteValue)
{
    decayCc_ = byteValue;
    const double ratio = static_cast<double>(byteValue) / 127.0;
    decayTimeMs_ = 1 + static_cast<unsigned long>(std::pow(ratio, 2.0) * 3000.0);
}

void StripeEngine::setRainbowTime(unsigned char byteValue)
{
    rainbowSpeedCc_ = byteValue;
    rainbowIntervalMs_ = 1 + byteValue;
}

void StripeEngine::setExplodeLevel(unsigned char byteValue)
{
    explodeLevelCc_ = byteValue;
}

void StripeEngine::setModeCc(unsigned char byteValue)
{
    modeCc_ = byteValue;
    isCcMode_ = byteValue > 60;
    needsToClearColors_ = true;
}

void StripeEngine::setCcLevel(unsigned char inCcLevel)
{
    levelCc_ = inCcLevel;
    ccLevel_ = velocityToLevelQuadratic(inCcLevel);
}

ControlParameters StripeEngine::controlParameters() const
{
    ControlParameters params{};
    params.decayCc = decayCc_;
    params.rainbowSpeedCc = rainbowSpeedCc_;
    params.explodeLevelCc = explodeLevelCc_;
    params.modeCc = modeCc_;
    params.levelCc = levelCc_;
    return params;
}

void StripeEngine::getStripeRgb(std::size_t stripeIndex, int& r, int& g, int& b) const
{
    r = g = b = 0;
    if (stripeIndex >= static_cast<std::size_t>(MidiMapping::kStripeCount))
        return;

    const int offset = static_cast<int>(stripeIndex) * kColorsPerStripe;
    r = colorStatus_[static_cast<std::size_t>(offset + kRedIndex)].level;
    g = colorStatus_[static_cast<std::size_t>(offset + kGreenIndex)].level;
    b = colorStatus_[static_cast<std::size_t>(offset + kBlueIndex)].level;

    if (r < 0)
        r = 0;
    if (g < 0)
        g = 0;
    if (b < 0)
        b = 0;
}

unsigned char StripeEngine::velocityToLevel(unsigned char velocity)
{
    return velocity <= 1 ? 0 : static_cast<unsigned char>(velocity * 2 + 1);
}

unsigned char StripeEngine::velocityToLevelQuadratic(unsigned char velocity)
{
    if (velocity <= 1)
        return 0;

    const float v = static_cast<float>(velocity) / 127.0f;
    return static_cast<unsigned char>(v * v * 255.0f);
}
