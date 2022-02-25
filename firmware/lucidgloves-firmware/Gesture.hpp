#pragma once

#include "Config.h"

#include "Finger.hpp"
#include "Encoding.hpp"
#include "Input.hpp"

class Gesture : public Input, public Encoder {
 public:
  Gesture(EncodingType type) : type(type), value(false) {}

  // Nothing to do here.
  void setup() override {}

  inline int getEncodedSize() const override {
    // Encode string size = single char + '\0'
    return 1;
  }

  int encode(char* output) const override {
    return snprintf(output, getEncodedSize(), "%c", value ? type : '\0');
  }

  bool isPressed() {
    return value;
  }

 private:
  EncodingType type;

 protected:
  bool value;
};

class GrabGesture : public Gesture {
 public:
  GrabGesture(const Finger* index, const Finger* middle,
              const Finger* ring, const Finger* pinky) :
    Gesture(EncodingType::GRAB),
    index(index), middle(middle), ring(ring), pinky(pinky)  {}

  // Grab gesture is pressed if the average all fingers is more than
  // halfway flexed.
  void readInput() override {
    value = (index->flexionValue() + middle->flexionValue() +
             ring->flexionValue() + pinky->flexionValue()) / 4 > ANALOG_MAX / 2;
  }

 private:
   const Finger* index;
   const Finger* middle;
   const Finger* ring;
   const Finger* pinky;
};

class TriggerGesture : public Gesture {
 public:
  TriggerGesture(const Finger* index_finger) :
    Gesture(EncodingType::TRIGGER), index_finger(index_finger) {}

  // Trigger gesture is pressed if the index finger is more than halfway flexed
  void readInput() override {
    value = index_finger->flexionValue() > ANALOG_MAX / 2;
  }

 private:
   const Finger* index_finger;
};

class PinchGesture : public Gesture {
 public:
  PinchGesture(const Finger* thumb, const Finger* index_finger) :
    Gesture(EncodingType::PINCH), thumb(thumb), index_finger(index_finger) {}

  // Pinch gesture is pressed if the average flex of the thumb and index is more than
  // halfway flexed.
  void readInput() override {
    // TODO: Do we need to divide both values here?
    value = (thumb->flexionValue() + index_finger->flexionValue()) / 2 > ANALOG_MAX / 2;
  }

  private:
   const Finger* thumb;
   const Finger* index_finger;
};