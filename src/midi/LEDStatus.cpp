#include "LEDStatus.h"

#include <cmath>

void LEDStatus::init(unsigned char inNote,
                     unsigned char inNoteAll,
                     unsigned char inNoteWhite,
                     unsigned char inNoteWhiteAll)
{
    note = inNote;
    noteAll = inNoteAll;
    noteWhite = inNoteWhite;
    noteWhiteAll = inNoteWhiteAll;
    clear();
}

void LEDStatus::clear()
{
    level = -1;
    fullLevel = 0;
    isOn = false;
}

void LEDStatus::applyLevel()
{
}

void LEDStatus::decayAndApply(unsigned long timePassedMs, unsigned long decayTimeMs)
{
    if (!isOn && level >= 0)
    {
        const unsigned long decay = timePassedMs - timeStampMs;
        const double ratio = static_cast<double>(decay) / static_cast<double>(decayTimeMs);
        level = fullLevel - static_cast<int>(std::lround(ratio * static_cast<double>(fullLevel)));
        applyLevel();
    }
}

void LEDStatus::update(bool status, int inLevel, unsigned long timePassedMs)
{
    isOn = status;
    if (status)
    {
        level = inLevel;
        fullLevel = static_cast<unsigned char>(inLevel);
    }
    else
    {
        timeStampMs = timePassedMs;
    }
    applyLevel();
}
