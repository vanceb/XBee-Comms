#include "xbee-comms.h"

# Macro to give number of elements in an array
#define NELEMS(x) (sizeof(x)/sizeof((x)[0]))

XBee-payload::XBeePayload() {
    clear();
}

void XBee-payload::clear() {
    cursor = 0;
}

int XBee-payload::set(uint8_t size, uint8_t* data) {
    clear();
    append(size, data)
}

int XBee-payload::append(uint8_t size, uint8_t* data) {
    if((cursor + size) < NELEMS(payload)) {
        for (int i=0; i<size; i++){
            payload[cursor++] = data[i];
        }
        return size;
    } else {
        return -1;
    }
}

int XBee-payload::append(uint8_t data) {
    if ((cursor + 1) < MAX_PAYLOAD) {
        payload[cursor++] = data;
        return 1;
    } else {
        return -1;
    }
}

int XBee-payload::append(uint16_t data) {
    if ((cursor + 2) < MAX_PAYLOAD) {
# Big Endian conversion into payload
        payload[cursor++] = (uint8_t)((data >> 8) & 0xFF);
        payload[cursor++] = (uint8_t)(data & 0xFF);
    }
}
int XBee-payload::append(uint32_t data) {
    if ((cursor + 4) < MAX_PAYLOAD) {
# Big Endian conversion into payload
        payload[cursor++] = (uint8_t)((data >> 24) & 0xFF);
        payload[cursor++] = (uint8_t)((data >> 16) & 0xFF);
        payload[cursor++] = (uint8_t)((data >> 8) & 0xFF);
        payload[cursor++] = (uint8_t)(data & 0xFF);
    }
}

uint8_t* XBee-payload::getPayload() {
    return payload;
}

uint8_t XBee-payload::getPayloadSize() {
    return cursor;
}

XBee-appPayload::clear() {
    cursor = 8;
}

void XBee-appPayload::setAppID(uint16_t value) {
    appID = value;
    uint8_t cursorPos = cursor;
    cursor = 0;
    append(value);
    cursor = cursorPos;
}

void XBee-appPayload::setMsgType (uint16_t value) {
    msgType = value;
    uint8_t cursorPos = cursor;
    cursor = 2;
    append(value);
    cursor = cursorPos;
}

void XBee-appPayload::setVersion (uint16_t value) {
    version = value;
    uint8_t cursorPos = 0;
    cursor = 4;
    append(value);
    cursor = cursorPos;
}

uint16_t XBee-appPayload::getAppID() {
    return appID;
}

uint16_t XBee-appPayload::getMsgType() {
    return msgType;
}

uint16_t XBee-appPayload::getVersion() {
    return version;
}

uint8_t XBee-appPayload::getDataLength() {
    return cursor - 8;
}

uint8_t* XBee-appPayload::getData() {
    return &payload[8];
}

XBee-comms XBee-comms::begin(Stream &serial);
int XBee-comms::transmit(XBee-payload data);
XBee-payload XBee-comms::receive();

