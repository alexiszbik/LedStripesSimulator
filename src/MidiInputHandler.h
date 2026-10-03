#pragma once

#include <RtMidi.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class MidiInputHandler
{
public:
    using MessageCallback = std::function<void(std::vector<unsigned char>)>;

    MidiInputHandler(std::string virtualPortName, MessageCallback callback);
    ~MidiInputHandler();

    MidiInputHandler(const MidiInputHandler&) = delete;
    MidiInputHandler& operator=(const MidiInputHandler&) = delete;

private:
    static void onMidiMessage(double /*deltaTime*/,
                              std::vector<unsigned char>* message,
                              void* userData);

    std::string virtualPortName_;
    MessageCallback callback_;
    std::unique_ptr<RtMidiIn> midiIn_;
};
