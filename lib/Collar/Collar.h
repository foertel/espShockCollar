// Collar.h

#pragma once

#include <Arduino.h>

#define START_PULSE_LEN_US 2200
#define START_PULSE_TOLLERANCE 100

// Enum Definitionen
enum collar_mode
{
  SHOCK = 1,
  VIBE = 2,
  BEEP = 3
};
enum collar_channel
{
  CH1 = 0,
  CH2 = 1,
  CH3 = 2
};

// Strukturdefinition
struct collar_message
{
  uint16_t id;
  collar_mode mode;
  collar_channel channel;
  uint8_t power;
};

class Collar
{
public:
  // Stuff for receiving
  using Callback = std::function<void(const collar_message *)>;
  // init to receive messages
  void setRxPin(uint8_t pin);
  void setMessageCallback(Callback message_callback);
  void startRx();

  // helpers to receive messages
  static void staticIsr();
  void isr();
  static void print_message(struct collar_message *msg);
  static Collar *instance;

  // init to send message
  void setTxPin(uint8_t pin);
  void initSender();
  void transmit(uint16_t id, collar_channel channel, collar_mode mode, uint8_t power);

private:
  // receiver
  uint8_t m_rx_pin;
  Callback m_message_callback;
  static const char *chan_to_str(collar_channel channel);
  static const char *mode_to_str(collar_mode mode);
  bool is_message_valid(uint8_t *buffer);
  void buffer_to_collar_message(uint8_t *buffer, collar_message *msg);

  // sender
  uint8_t m_tx_pin;
  void tx_start();
  void tx_bit(bool one);
  void tx_byte(uint8_t val);
  void tx_buffer(uint8_t *buf, uint8_t buf_len);
};
