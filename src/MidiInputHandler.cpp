#include "MidiInputHandler.h"

MidiInputHandler::MidiInputHandler(std::string virtualPortName, MessageCallback callback)
    : virtualPortName_(std::move(virtualPortName))
    , callback_(std::move(callback))
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

void MidiInputHandler::onMidiMessage(double /*deltaTime*/,
                                     std::vector<unsigned char>* message,
                                     void* userData)
{
    if (message == nullptr || userData == nullptr)
        return;

    auto* handler = static_cast<MidiInputHandler*>(userData);
    if (handler->callback_)
        handler->callback_(*message);
}
