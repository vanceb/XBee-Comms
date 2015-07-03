#ifndef XBEE_COMMS
#define XBEE_COMMS

#include "XBee.h"

#define MAX_PAYLOAD 100

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
        uint16_t getAppID();
        uint16_t getMsgType();
        uint16_t getVersion();
        uint8_t getDataLength();
        uint8_t* getData();

}
class XBee-comms {
    private:
        XBee xbee;

    protected:

    public:
        XBee-comms begin(Stream &serial);
        int transmit(XBee-payload data);
        XBee-payload receive();

}
#endif
