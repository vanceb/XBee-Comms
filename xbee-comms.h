#ifndef XBEE_COMMS
#define XBEE_COMMS

#include "XBee.h"

#define MAX_PAYLOAD 100

#define TELEMETRY_APP_ID 0x0573
#define TX_STATUS_MSG 0x0001
#define TX_STATUS_VERSION 0x0001
#define TX_STATUS_UPDATE_PERIOD 60000

#define TX_CTS_TIMEOUT 1000
#define TX_RESPONSE_TIMEOUT 5000

class XBee-payload {
private:
    uint8_t payload[MAX_PAYLOAD];
    uint8_t cursor;

protected:

public:
    void clear();
    int set(uint8_t size, uint8_t* data);
    int append(uint8_t size, uint8_t* data);
    int append(uint8_t data);
    int append(uint16_t data);
    int append(uint32_t data);

    uint8_t* getPayload();
    uint8_t getPayloadSize();
}


class XBee-appPayload: Public XBee-payload {
private:
    uint16_t appID;
    uint16_t msgType;
    uint16_t version;
    uint16_t length;

protected:

public:
    void setAppID(uint16_t value);
    void setMsgType (uint16_t value);
    void setVersion (uint16_t value);
    int setData(uinta_t length, uint8_t* data);
    uint16_t getAppID();
    uint16_t getMsgType();
    uint16_t getVersion();
    uint8_t getDataLength();
    uint8_t* getData();

}


class XBee-comms {
private:
    static XBee xbee;
    static int sleepPin;
    static int ctsPin;

    unsigned long nextTxStatus;

    // Create a payload item for our TxStatus updates
    XBee-appPayload txStatusPayload;

    // Define a sctructure to hold the TX Status data
    typedef struct tagTxStatus {
        // Time since restart
        uint32_t millis;
        // Tx Failures
        uint16_t tx_packets;
        uint16_t tx_local_cts_timeout;
        uint16_t tx_local_timeout;
        uint16_t tx_cca_failure;
        uint16_t tx_invalid_dest;
        uint16_t tx_network_ack;
        uint16_t tx_not_joined;
        uint16_t tx_self_addr;
        uint16_t tx_addr_not_found;
        uint16_t tx_no_route;
        uint16_t tx_payload_too_big;
        uint16_t tx_other1;
        uint16_t tx_other2;
    } TxStatus;

protected:
    sleep();
    wake();
    resetTxStatus();
    sendTxStatus();

public:
    XBee-comms begin(Stream &serial, int sleep_pin, int cts_pin);
    XBee-comms begin(Stream &serial, int sleep_pin);
    XBee-comms begin(Stream &serial);

    int transmit(XBee-payload data);
    XBee-payload receive();

}




#endif
