#include <Net.h>

RF24 _nrf24l01radio (NRF24L01_CE_PIN, NRF24L01_CSN_PIN);

void Net::getBroadcast (byte *netbroadcast) {

  memcpy (netbroadcast+1, netAddr, 4);
  netbroadcast[0] = 0;
}

RF24 Net::getRadio () {
  return _nrf24l01radio;
}

/**
 * Return a pointer to the source address
 * NOTE - The memory used by this address will be reused for subsequent frames.
 *
 * @return Pointer to a 5 byte array that contains the source address.
 */
byte* Net::getSourceAddress () {
  return sourceAddress;
}

/**
 * Return a pointer to the destination address
 * NOTE - The memory used by this address will be reused for subsequent frames.
 *
 * @return Pointer to a 5 byte array that contains the destination address.
 */
byte* Net::getDestinationAddress () {
  return destinationAddress;
}


/**
 * Return a pointer to the data section of the frame.
 * For a v0 frame this data segment will be 29 bytes long. Note this may change in the
 * future. You should call getDataSize () in order to determine the
 * size of the data segment.
 * NOTE - The memory used by this data segment will be reused to retrieve
 * the next frame.
 *
 * @return Pointer to a byte array containing the data segment of the frame.
 *
 */
void* Net::getData () {
  return (void*)&netData.data;
}


/**
 * Returns the size of the data segment for this frame.
 * @return The size of the data segment for this frame.
 */
int Net::getDataSize () {
  // v0 has a 28 byte data size.
  return (int)netData.size;
}

/**
 *
 *Blank constructor. Do nothing.
 * 
 */
Net::Net () {
  frameReceived = 0;
}


/**
 * Configure the network stack
 *
 * @param netAddress The network address to use. The host address will be automatically configured.
 */
void Net::setup (byte netAddress[4], void (*netCallback)(Net *frame)){
  byte netbroadcast[5];
  byte hostAddr[5];

//  println (0);

  memcpy (netAddr, netAddress, 4);
  memcpy (hostAddr, netAddress, 4);

  frameReceived = netCallback;

  getRadio ().begin();
  // hardcoded channel for each of use
  getRadio ().setChannel(0x4c);
  getRadio ().setRetries(15,15);
  getRadio ().setDataRate(RF24_250KBPS);

// dynamic payloads didn't work during tests
// use static size of 32 bytes
//  getRadio ().enableDynamicPayloads ();
  getRadio ().setPayloadSize(32);

  memcpy (netbroadcast+1, netAddress, 4);
  netbroadcast[0] = 0;
//  netbroadcast[1] = 0;
//  netbroadcast[2] = 0;
//  netbroadcast[3] = 0;
//  netbroadcast[4] = 0;

//  getRadio ().openReadingPipe(1,netbroadcast);
// don't acknowledge the network broadcast address
// don't want to create a network feedback loop of
// acknowledgements.

// for some reason the writing pipe need auto ack turned on.
  getRadio ().setAutoAck (0);
  getRadio ().enableDynamicAck ();
//  getRadio ().setAutoAck(0, 1);

// listen on the broadcast pipe
  getRadio ().openReadingPipe(1,netbroadcast);
  getRadio ().openWritingPipe(netbroadcast);
  getRadio ().setAutoAck(1, 0);
  getRadio ().setAutoAck(0, 0);

// This will be our host address
// we should use the Duplication Address Detection system
// for now just hardcoded.
  hostAddress = 0;
  boolean addrInUse = 0;
  byte proposedHostAddress = 20;

  getRadio ().startListening ();

  do {
    // randomise
    struct duplicateAddressDetection rdad;
    rdad.requestAddress = proposedHostAddress;
    rdad.flags = DAD_REQUEST;

    addrInUse = 0;

    for (int i = 0; (i < 20) && (addrInUse == 0); i ++) {
      write (netbroadcast, &rdad, sizeof (rdad), PORT_DAD);
      delay (20);

      for (int j = 0; (j < 10) && (addrInUse == 0); j ++) {
        if (getRadio ().available ()) {
          getRadio ().read (&netData, 32);
          if (getPort () == PORT_DAD) {
            memcpy (&rdad, getData (), sizeof (rdad));
            // address in use
            if ((rdad.requestAddress == proposedHostAddress) &&
            (rdad.flags == DAD_ADDR_INUSE)){
              addrInUse = 1;
            }
          }
        }
      }
    }
    if (addrInUse == 1) {
      proposedHostAddress = random (20, 255);
    }
  } while (addrInUse == 1);

  hostAddress = proposedHostAddress;
  hostAddr[0] = proposedHostAddress;
  getRadio ().openReadingPipe(2,hostAddr);

  getRadio ().setAutoAck(2, 1);

// reset the radio
  getRadio ().stopListening ();
  getRadio ().startListening ();
}

void Net::updateComms () {
  if (getRadio ().available ()) {
    // dynamic paylod size doesn't work in tests
//    int dataSize = getRadio ().getDynamicPayloadSize ();
      getRadio ().read (&netData, 32);
// Dynamic payload didn't work network data is always 32 bytes.

    int port = getPort ();
    switch (port) {
      case 0: // call the user callback
          frameReceived (this);
        break;
      case PORT_DAD: // Duplicate address detection
          if (netData.destination == 0) { // only take part if it was sent to the network broadcast address
            struct duplicateAddressDetection dad;
            memcpy (&dad, getData (), sizeof (dad));
            if (hostAddress == dad.requestAddress) {
              // we are using the proposed address
              struct duplicateAddressDetection rdad;
              rdad.requestAddress = hostAddress;
              rdad.flags = DAD_ADDR_INUSE;

              write (getSourceAddress (), &rdad, sizeof (rdad), PORT_DAD);
            }
          }
        break;
      default: // no idea what to do this this packet.
        break;
     }

  }
}

int Net::getPort () {
  int port = (netData.flags & NET_PORT_MASK);
  return port;
}

int Net::getVersion () {
  int version = ((netData.flags & NET_VERSION_MASK) >> 4);
  return version;
}

int Net::write (byte destination[5], void *data, int size) {
  write (destination, data, size, 0);
}

int Net::write (byte destination[5], void *data, int size, int port) {
  int result = 0;
  struct networkFrame nf;

  memset (&nf, 0, sizeof (nf));
  nf.flags = 0;
  nf.source = hostAddress;
  nf.destination = destination[0];
  int s = 28;
  if (size < s) {
    s = size;
  }
  nf.size = s;
  nf.flags = (NET_VERSION << 4) | port;
  memcpy (nf.data, data, s);

  getRadio ().stopListening ();
  getRadio ().openWritingPipe (destination);
  getRadio ().setAutoAck (1);
  result = getRadio ().write (&nf, sizeof (nf));
  getRadio ().startListening ();
  return result;
}

byte Net::getMyAddress () {
  return hostAddress;
}
