#ifndef NET_H
#define NET_H

#include <Arduino.h>

#include "SPI.h"
#include "nRF24L01.h"
#include "RF24.h"

#define NRF24L01_CE_PIN 8
#define NRF24L01_CSN_PIN 7

#define NRF24L01_CHANNEL 0x4c

#define NET_VERSION (0)
#define NET_VERSION_MASK (0b11110000)
#define NET_PORT_MASK (0b00001111)

#define GLOBAL_BROADCAST_ADDRESS 0

#define DAD_REQUEST (1)
#define DAD_ADDR_INUSE (1 << 1)

#define PORT_USER 0
#define PORT_DAD 1

class Net {
  private:
    struct duplicateAddressDetection {
      byte requestAddress;
      byte flags;
    };
    struct networkFrame {
      byte flags;
      byte source;
      byte destination;
      byte size;
      byte data[32 - 4];
    } netData;
    int getPort ();
    int getVersion ();
    int write (byte destination[5], void *data, int size, int port);

  public:
//    int packets;
    Net ();
    void (*frameReceived)(Net *frame);

    RF24 getRadio ();
    byte hostAddress;
    byte netAddr[4];
    byte sourceAddress[5];
    byte destinationAddress[5];
    byte* getSourceAddress ();
    byte* getDestinationAddress ();
    byte getMyAddress ();
    void getBroadcast (byte *broadcast);
    void* getData ();
    void setup (byte netAddress[4], void (*callback)(Net *frame));
    int getDataSize ();
    void updateComms ();
    int write (byte destination[5], void *data, int size);
    void (*println) (char *);
};

#endif // End of NET_H
