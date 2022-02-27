#include "Config.h"

#include "Button.hpp"
#include "Calibration.hpp"
#include "Finger.hpp"
#include "Gesture.hpp"
#include "Input.hpp"
#include "JoyStick.hpp"
#include "LED.hpp"
#include "Output.hpp"

#include "ICommunication.hpp"
#if COMMUNICATION == COMM_SERIAL
  #include "SerialCommunication.hpp"
#elif COMMUNICATION == COMM_BTSERIAL
  #include "SerialBTCommunication.hpp"
#endif

#define ALWAYS_CALIBRATING CALIBRATION_LOOPS == -1

ICommunication* comm;
int calibration_count = 0;

LED led(PIN_LED);


#if USING_CALIB_PIN
  // This button is referenced directly by the FW, so we need a pointer to it outside
  // the list of buttons.
  Button calibration_button(Encoder::Type::CALIB, PIN_CALIB, INVERT_CALIB);
#endif

Button* buttons[BUTTON_COUNT] = {
  new Button(Encoder::Type::A_BTN, PIN_A_BTN, INVERT_A),
  new Button(Encoder::Type::B_BTN, PIN_B_BTN, INVERT_B),
  new Button(Encoder::Type::MENU, PIN_MENU_BTN, INVERT_MENU),
  #if ENABLE_JOYSTICK
    new Button(Encoder::Type::JOY_BTN, PIN_JOY_BTN, INVERT_JOY),
  #endif

  #if !TRIGGER_GESTURE
    new Button(Encoder::Type::TRIGGER, PIN_TRIG_BTN, INVERT_TRIGGER),
  #endif
  #if !GRAB_GESTURE
    new Button(Encoder::Type::GRAB, PIN_GRAB_BTN, INVERT_GRAB),
  #endif
  #if !PINCH_GESTURE
    new Button(Encoder::Type::PINCH, PIN_PNCH_BTN, INVERT_PINCH),
  #endif
  #if USING_CALIB_PIN
    &calibration_button,
  #endif
};

#if !ENABLE_SPLAY
  #if ENABLE_THUMB
  Finger finger_thumb(Encoder::Type::THUMB, Decoder::Type::THUMB, PIN_THUMB, PIN_THUMB_MOTOR);
  #endif
  Finger finger_index(Encoder::Type::INDEX, Decoder::Type::INDEX, PIN_INDEX, PIN_INDEX_MOTOR);
  Finger finger_middle(Encoder::Type::MIDDLE, Decoder::Type::MIDDLE, PIN_MIDDLE, PIN_MIDDLE_MOTOR);
  Finger finger_ring(Encoder::Type::RING, Decoder::Type::RING, PIN_RING, PIN_RING_MOTOR);
  Finger finger_pinky(Encoder::Type::PINKY, Decoder::Type::PINKY, PIN_PINKY, PIN_PINKY_MOTOR);
#else
  #if ENABLE_THUMB
  SplayFinger finger_thumb(Encoder::Type::THUMB, Decoder::Type::THUMB, PIN_THUMB, PIN_THUMB, PIN_THUMB_MOTOR);
  #endif
  SplayFinger finger_index(Encoder::Type::INDEX, Decoder::Type::INDEX, PIN_INDEX, PIN_INDEX_SPLAY, PIN_INDEX_MOTOR);
  SplayFinger finger_middle(Encoder::Type::MIDDLE, Decoder::Type::MIDDLE, PIN_MIDDLE, PIN_MIDDLE_SPLAY, PIN_MIDDLE_MOTOR);
  SplayFinger finger_ring(Encoder::Type::RING, Decoder::Type::RING, PIN_RING, PIN_RING_SPLAY, PIN_RING_MOTOR);
  SplayFinger finger_pinky(Encoder::Type::PINKY, Decoder::Type::PINKY, PIN_PINKY, PIN_PINKY_SPLAY, PIN_PINKY_MOTOR);
#endif

Finger* fingers[FINGER_COUNT] = {
  #if ENABLE_THUMB
    &finger_thumb,
  #endif
  &finger_index, &finger_middle, &finger_ring, &finger_pinky
};

JoyStickAxis* joysticks[JOYSTICK_COUNT] = {
  #if ENABLE_JOYSTICK
    new JoyStickAxis(Encoder::Type::JOY_X, PIN_JOY_X, JOYSTICK_DEADZONE, INVERT_JOY_X),
    new JoyStickAxis(Encoder::Type::JOY_Y, PIN_JOY_Y, JOYSTICK_DEADZONE, INVERT_JOY_Y)
  #endif
};

Gesture* gestures[GESTURE_COUNT] = {
  #if TRIGGER_GESTURE
    new TriggerGesture(&finger_index),
  #endif
  #if GRAB_GESTURE
    new GrabGesture(&finger_index, &finger_middle, &finger_ring, &finger_pinky),
  #endif
  #if PINCH_GESTURE
    new PinchGesture(&finger_thumb, &finger_index)
  #endif
};

// These are composite lists of the above list for a cleaner loop.
Calibrated* calibrators[CALIBRATED_COUNT];
Encoder* encoders[INPUT_COUNT];
Input* inputs[INPUT_COUNT];
Output* outputs[OUTPUT_COUNT];
Decoder* decoders[OUTPUT_COUNT];

char* encoded_output_string;

void setup() {
  #if COMMUNICATION == COMM_SERIAL
    comm = new SerialCommunication();
  #elif COMMUNICATION == COMM_BTSERIAL
    comm = new BTSerialCommunication();
  #endif

  comm->start();

  // Register the inputs and encoders.
  size_t next_input = 0;
  for (size_t i = 0; i < BUTTON_COUNT; next_input++, i++) {
    encoders[next_input] = buttons[i];
    inputs[next_input] = buttons[i];
  }

  for (size_t i = 0; i < FINGER_COUNT; next_input++, i++) {
    encoders[next_input] = fingers[i];
    inputs[next_input] = fingers[i];
  }

  for (size_t i = 0; i < JOYSTICK_COUNT; next_input++, i++) {
    encoders[next_input] = joysticks[i];
    inputs[next_input] = joysticks[i];
  }

  for (size_t i = 0; i < GESTURE_COUNT; next_input++, i++) {
    // Gestures should be at the end of the list since their inputs
    // are based on other inputs.
    encoders[next_input] = gestures[i];
    inputs[next_input] = gestures[i];
  }

  // Register the calibrated inputs
  size_t next_calibrator = 0;
  for (size_t i = 0; i < FINGER_COUNT; next_calibrator++, i++) {
    calibrators[next_calibrator] = fingers[i];
  }

  // Register the outputs and decoders
  int next_output = 0;
  #if USING_FORCE_FEEDBACK
    for (size_t i = 0; i < FINGER_COUNT; next_input++, i++) {
      decoders[next_output] = fingers[i];
      outputs[next_output] = fingers[i];
    }
  #endif

  // Figure out needed size for the output string.
  int string_size = 0;
  for(size_t i = 0; i < INPUT_COUNT; i++) {
    string_size += encoders[i]->getEncodedSize();
  }

  // Add 1 new line and 1 for the null terminator.
  encoded_output_string = new char[string_size + 1 + 1];

  // Setup all the inputs.
  for (size_t i = 0; i < INPUT_COUNT; i++) {
    inputs[i]->setupInput();
  }

  // Setup all the outputs.
  for (size_t i = 0; i < OUTPUT_COUNT; i++) {
    outputs[i]->setupOutput();
  }

  // Setup the LED.
  // TODO: Should this just be in the output list?
  //       If the driver can send an LED state in the future
  //       then it will need to be a decoder and an output.
  led.setupOutput();
}

void loop() {
  if (!comm->isOpen()){
    // Connection to FW not ready, blink the LED to indicate no connection.
    led.writeOutput(LED::State::BLINK_STEADY);
  } else {
    // All is good, LED on to indicate a good connection.
    led.writeOutput(LED::State::ON);
  }

  char received_bytes[100];
  if (comm->hasData() && comm->readData(received_bytes)) {
    for (size_t i = 0; i < OUTPUT_COUNT; i++) {
      // Decode the update and write it to the output.
      outputs[i]->writeOutput(decoders[i]->decode(received_bytes));
    }
  }

  // Update all the inputs
  for (int i = 0; i < INPUT_COUNT; i++) {
    inputs[i]->readInput();
  }

  // Setup calibration to be activated in the next loop iteration.
  #if USING_CALIB_PIN
    bool calibrate_pressed = calibration_button.isPressed();
  #else
    bool calibrate_pressed = false;
  #endif

  // Notify the calibrators to turn on.
  if (calibrate_pressed) {
    calibration_count = 0;
    for (size_t i = 0; i < CALIBRATED_COUNT; i++) {
      calibrators[i]->enableCalibration();
    }
  }

  if (calibration_count < CALIBRATION_LOOPS || ALWAYS_CALIBRATING){
    // Keep calibrating for one at least one more loop.
    calibration_count++;
  } else {
    // Calibration is done, notify the calibrators
    for (size_t i = 0; i < CALIBRATED_COUNT; i++) {
      calibrators[i]->disableCalibration();
    }
  }

  // Encode all of the outputs to a single string.
  encodeAll(encoded_output_string, encoders, INPUT_COUNT);

  // Send the string to the communication handler.
  comm->output(encoded_output_string);

  delay(LOOP_TIME);
}
