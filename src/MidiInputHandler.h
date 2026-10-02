#pragma once

#include <RtMidi.h>

#include <memory>
#include <string>

class MidiInputHandler
{
public:
    explicit MidiInputHandler(std::string virtualPortName);
    ~MidiInputHandler();

    MidiInputHandler(const MidiInputHandler&) = delete;
    MidiInputHandler& operator=(const MidiInputHandler&) = delete;

    void run();

private:
    static void onMidiMessage(double /*deltaTime*/,
                              std::vector<unsigned char>* message,
                              void* userData);

    void handleMessage(const std::vector<unsigned char>& message) const;
    static void logNoteOn(int note, int velocity);
    static void logNoteOff(int note);

    std::string virtualPortName_;
    std::unique_ptr<RtMidiIn> midiIn_;
};
