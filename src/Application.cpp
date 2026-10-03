#include "Application.h"

#include "midi/MidiMapping.h"

#include <cstdlib>

namespace
{
constexpr unsigned char kStatusMask = 0xF0;
constexpr unsigned char kChannelMask = 0x0F;
constexpr unsigned char kNoteOffMask = 0x80;
constexpr unsigned char kNoteOnMask = 0x90;
constexpr unsigned char kControlChangeMask = 0xB0;
} // namespace

Application::Application(std::string virtualPortName)
    : virtualPortName_(std::move(virtualPortName))
    , midiInput_(virtualPortName_, [this](std::vector<unsigned char> message) {
          enqueueRawMessage(std::move(message));
      })
    , startTime_(std::chrono::steady_clock::now())
{
    std::srand(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
    stripeEngine_.setup();
}

Application::~Application() = default;

int Application::run()
{
    if (!raylibUi_.init())
        return 1;

    while (!raylibUi_.shouldClose())
    {
        const auto now = std::chrono::steady_clock::now();
        const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime_).count();

        processPendingMessages();
        stripeEngine_.tick(static_cast<unsigned long>(elapsedMs));
        raylibUi_.draw(stripeEngine_, virtualPortName_);
        raylibUi_.pollInput();
    }

    raylibUi_.shutdown();
    return 0;
}

void Application::enqueueRawMessage(std::vector<unsigned char> message)
{
    if (message.empty())
        return;

    std::lock_guard lock(queueMutex_);
    pendingMessages_.push_back(std::move(message));
}

void Application::processPendingMessages()
{
    std::deque<std::vector<unsigned char>> localQueue;

    {
        std::lock_guard lock(queueMutex_);
        localQueue.swap(pendingMessages_);
    }

    while (!localQueue.empty())
    {
        handleRawMessage(localQueue.front());
        localQueue.pop_front();
    }
}

void Application::handleRawMessage(const std::vector<unsigned char>& message)
{
    if (message.size() < 2)
        return;

    const auto status = message[0];
    const auto messageType = static_cast<unsigned char>(status & kStatusMask);
    const int channel = static_cast<int>((status & kChannelMask) + 1);

    if (messageType == kNoteOnMask || messageType == kNoteOffMask)
    {
        const int note = static_cast<int>(message[1]);
        const int velocity = message.size() > 2 ? static_cast<int>(message[2]) : 0;

        if (messageType == kNoteOnMask)
            handleNote(channel, note, velocity > 0 ? velocity : 0);
        else
            handleNote(channel, note, 0);

        return;
    }

    if (messageType == kControlChangeMask && message.size() > 2)
    {
        const int control = static_cast<int>(message[1]);
        const int value = static_cast<int>(message[2]);
        handleControlChange(channel, control, value);
    }
}

bool Application::isTargetChannel(int channel)
{
    return channel == MidiMapping::kMidiChannel;
}

void Application::handleNote(int channel, int note, int velocity)
{
    if (!isTargetChannel(channel))
        return;

    stripeEngine_.setNote(static_cast<unsigned char>(note), static_cast<unsigned char>(velocity));
}

void Application::handleControlChange(int channel, int control, int value)
{
    if (!isTargetChannel(channel))
        return;

    switch (control)
    {
    case MidiMapping::kCcDecay:
        stripeEngine_.setDecayTime(static_cast<unsigned char>(value));
        break;
    case MidiMapping::kCcRainbowSpeed:
        stripeEngine_.setRainbowTime(static_cast<unsigned char>(value));
        break;
    case MidiMapping::kCcExplodeLevel:
        stripeEngine_.setExplodeLevel(static_cast<unsigned char>(value));
        break;
    case MidiMapping::kCcMode:
        stripeEngine_.setModeCc(static_cast<unsigned char>(value));
        break;
    case MidiMapping::kCcLevel:
        stripeEngine_.setCcLevel(static_cast<unsigned char>(value));
        break;
    default:
        break;
    }
}
