#pragma once

#include "MidiInputHandler.h"
#include "midi/StripeEngine.h"
#include "ui/RaylibUi.h"

#include <chrono>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

class Application
{
public:
    explicit Application(std::string virtualPortName);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int run();

private:
    void enqueueRawMessage(std::vector<unsigned char> message);
    void processPendingMessages();
    void handleRawMessage(const std::vector<unsigned char>& message);
    void handleNote(int channel, int note, int velocity);
    void handleControlChange(int channel, int control, int value);
    static bool isTargetChannel(int channel);

    std::string virtualPortName_;
    StripeEngine stripeEngine_;
    RaylibUi raylibUi_;
    MidiInputHandler midiInput_;

    std::mutex queueMutex_;
    std::deque<std::vector<unsigned char>> pendingMessages_;

    std::chrono::steady_clock::time_point startTime_;
};
