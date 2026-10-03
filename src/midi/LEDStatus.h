#pragma once

struct LEDStatus
{
    void init(unsigned char inNote,
              unsigned char inNoteAll,
              unsigned char inNoteWhite,
              unsigned char inNoteWhiteAll);

    void clear();
    void applyLevel();
    void decayAndApply(unsigned long timePassedMs, unsigned long decayTimeMs);
    void update(bool status, int inLevel, unsigned long timePassedMs);

    unsigned char note = 0;
    unsigned char noteAll = 0;
    unsigned char noteWhite = 0;
    unsigned char noteWhiteAll = 0;

    int level = 0;
    unsigned char fullLevel = 0;
    bool isOn = false;
    unsigned long timeStampMs = 0;
};
