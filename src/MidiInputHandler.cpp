#include "MidiInputHandler.h"

#include <chrono>
#include <iostream>
#include <thread>

namespace
{
constexpr unsigned char kNoteOffMask = 0x80;
constexpr unsigned char kNoteOnMask = 0x90;
constexpr unsigned char kStatusMask = 0xF0;
} // namespace

MidiInputHandler::MidiInputHandler(std::string virtualPortName)
    : virtualPortName_(std::move(virtualPortName))
    , midiIn_(std::make_unique<RtMidiIn>(RtMidi::UNSPECIFIED, virtualPortName_))
{
    midiIn_->ignoreTypes(/*midiSysex=*/true, /*midiTime=*/true, /*midiSense=*/true);
    midiIn_->setCallback(&MidiInputHandler::onMidiMessage, this);
    midiIn_->openVirtualPort(virtualPortName_);
}

MidiInputHandler::~MidiInputHandler()
{
    if (midiIn_ != nullptr && midiIn_->isPortOpen())
        midiIn_->closePort();
}

void MidiInputHandler::run()
{
    std::cout << "Virtual MIDI input \"" << virtualPortName_
              << "\" is open. Send notes to this port (Ctrl+C to quit).\n"
              << std::flush;

    for (;;)
        std::this_thread::sleep_for(std::chrono::hours(24));
}

void MidiInputHandler::onMidiMessage(double /*deltaTime*/,
                                     std::vector<unsigned char>* message,
                                     void* userData)
{
    if (message == nullptr || userData == nullptr)
        return;

    auto* handler = static_cast<MidiInputHandler*>(userData);
    handler->handleMessage(*message);
}

void MidiInputHandler::handleMessage(const std::vector<unsigned char>& message) const
{
    if (message.size() < 2)
        return;

    const auto status = message[0];
    const auto messageType = static_cast<unsigned char>(status & kStatusMask);

    if (messageType != kNoteOnMask && messageType != kNoteOffMask)
        return;

    const int note = static_cast<int>(message[1]);
    const int velocity = message.size() > 2 ? static_cast<int>(message[2]) : 0;

    if (messageType == kNoteOnMask) {
        if (velocity > 0) {
            logNoteOn(note, velocity);
        } else {
            logNoteOff(note);
        }
    } else if (messageType == kNoteOffMask) {
        logNoteOff(note);
    }
}

void MidiInputHandler::logNoteOn(int note, int velocity)
{
    std::cout << "MIDI note On: " << note << '\n' << std::flush;
}

void MidiInputHandler::logNoteOff(int note)
{
    std::cout << "MIDI note Off: " << note << '\n' << std::flush;
}
