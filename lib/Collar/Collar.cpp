#include <Collar.h>

Collar *Collar::instance = nullptr;

void Collar::setRxPin(uint8_t pin)
{
    m_rx_pin = pin;
}

void Collar::setTxPin(uint8_t pin)
{
    m_tx_pin = pin;
}

void Collar::setMessageCallback(Callback message_callback)
{
    m_message_callback = message_callback;
}

void Collar::startRx()
{
    pinMode(m_rx_pin, INPUT_PULLUP);

    instance = this;

    // Interrupt registrieren
    attachInterrupt(digitalPinToInterrupt(m_rx_pin), Collar::staticIsr, RISING);
}

void Collar::initSender()
{
    pinMode(m_tx_pin, OUTPUT);
    digitalWrite(m_tx_pin, LOW);
}

void Collar::isr()
{
    static unsigned long rx_micros = 0;
    static uint8_t pulse_count = 0;
    delayMicroseconds(500); // BAD: really shouldn't do this in an ISR
    uint8_t val = digitalRead(m_rx_pin);

    static struct collar_message _rx_msg;

    static uint8_t buffer[5];        // expecting to receive 5 bytes
    static uint8_t byte_postion = 0; // keep track of current byte being received
    static uint8_t bit_position = 0; // keep track of expected next bit postion in byte

    // look for tranmission start state (rising edge->rising edge of ~2.2ms)
    int last_pulse_len = micros() - rx_micros;
    if ((last_pulse_len > START_PULSE_LEN_US - START_PULSE_TOLLERANCE) &&
        (last_pulse_len < START_PULSE_LEN_US + START_PULSE_TOLLERANCE))
    {
        pulse_count = 0;

        byte_postion = 0;
        bit_position = 0;
        memset(buffer, 0, sizeof(buffer));
    }

    if (byte_postion < sizeof(buffer))
    {
        if (val)
            buffer[byte_postion] |= (1 << (7 - bit_position));

        if (++bit_position >= 8)
        {
            bit_position = 0;
            byte_postion++;
        }

        if (byte_postion >= sizeof(buffer))
        {
            if (is_message_valid(buffer))
            {
                buffer_to_collar_message(buffer, &_rx_msg);
                m_message_callback(&_rx_msg);
            }
        }
    }

    rx_micros = micros();

    if (pulse_count < 50)
    {
        pulse_count++;
    }
}

IRAM_ATTR void Collar::staticIsr()
{
    if (instance)
    {
        instance->isr();
    }
}

bool Collar::is_message_valid(uint8_t *buffer)
{
    // calculate checksum
    uint8_t chk = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        chk += buffer[i];
    }

    // validate checksum is correct
    if (chk != buffer[4])
        return false;

    return true;
}

void Collar::buffer_to_collar_message(uint8_t *buf, collar_message *msg)
{
    // wipe old message
    memset(msg, 0, sizeof(collar_message));

    // bytes 0&1 = ID
    memcpy(&msg->id, buf, 2);

    // byte 2 = mode & channel
    msg->mode = (collar_mode)(buf[2] & 0x0F);
    msg->channel = (collar_channel)((buf[2] & 0xF0) >> 4);

    // byte 3 = power
    msg->power = buf[3];

    // byte 4 = checksum
}

void Collar::print_message(struct collar_message *msg)
{
    Serial.print("ID:\t0x");
    Serial.println(msg->id, HEX);

    Serial.print("Chan.:\t");
    Serial.println(Collar::chan_to_str(msg->channel));

    Serial.print("Mode:\t");
    Serial.println(Collar::mode_to_str(msg->mode));

    Serial.print("Power:\t");
    Serial.println(msg->power);
}

const char *Collar::chan_to_str(collar_channel channel)
{
    switch (channel)
    {
    case CH1:
        return "CH1";

    case CH2:
        return "CH2";

    case CH3:
        return "CH3";

    default:
        return "CH?";
    }
}

const char *Collar::mode_to_str(collar_mode mode)
{
    switch (mode)
    {
    case SHOCK:
        return "Shock";

    case VIBE:
        return "Vibrate";

    case BEEP:
        return "Beep";

    default:
        return "MODE?";
    }
}

void Collar::transmit(uint16_t id, collar_channel channel, collar_mode mode, uint8_t power)
{
    uint8_t txbuf[4]; // bytes 0+1=ID, 2=mode&channel, 3=power

    memcpy(txbuf, &id, 2);
    txbuf[2] = ((channel << 4) | mode);

    // Power levels >99 are ignored by the collar
    if (power > 99)
        txbuf[3] = 99;
    else
        txbuf[3] = power;

    // The collar seems to expect to receive the same message 3 times
    tx_buffer(txbuf, sizeof(txbuf));
    tx_buffer(txbuf, sizeof(txbuf));
    tx_buffer(txbuf, sizeof(txbuf));
}

void Collar::tx_buffer(uint8_t *buf, uint8_t buf_len)
{
    uint8_t check = 0;
    tx_start();

    for (int n = 0; n < buf_len; n++)
    {
        check += buf[n];
        tx_byte(buf[n]);
    }
    tx_byte(check);

    // Not sure why this is needed. The original remote sends it, and it seems temperamental without it, so whatever...
    tx_bit(0);
}

void Collar::tx_start()
{
    digitalWrite(m_tx_pin, HIGH);
    delayMicroseconds(1550);
    digitalWrite(m_tx_pin, LOW);
    delayMicroseconds(660);
}

void Collar::tx_bit(bool one)
{
    if (one)
    {
        digitalWrite(m_tx_pin, HIGH);
        delayMicroseconds(830);
        digitalWrite(m_tx_pin, LOW);
        delayMicroseconds(200);
    }
    else
    {
        digitalWrite(m_tx_pin, HIGH);
        delayMicroseconds(360);
        digitalWrite(m_tx_pin, LOW);
        delayMicroseconds(670);
    }
}

void Collar::tx_byte(uint8_t val)
{
    for (int n = 7; n >= 0; n--)
        tx_bit(val & (1 << n));
}