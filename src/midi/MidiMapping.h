#pragma once

namespace MidiMapping
{
constexpr int kStripeCount = 6;
constexpr int kMidiChannel = 15; // 1-based, matches MIDI-LED-Strip-Controller

constexpr int kRainbowNote = 58;
constexpr int kExplodeNote = 59;

constexpr int kRedNoteAll = 60;
constexpr int kNoteStart = 64;

constexpr int kCcDecay = 3;
constexpr int kCcRainbowSpeed = 4;
constexpr int kCcExplodeLevel = 5;
constexpr int kCcMode = 6;
constexpr int kCcLevel = 7;
} // namespace MidiMapping
