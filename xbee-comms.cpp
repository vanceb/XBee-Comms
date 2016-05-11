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
        return 1;
    } else {
        return -1;
    }
}

int XBee-payload::append(uint32_t data) {
    if ((cursor + 4) < MAX_PAYLOAD) {
        # Big Endian conversion into payload
        payload[cursor++] = (uint8_t)((data >> 24) & 0xFF);
        payload[cursor++] = (uint8_t)((data >> 16) & 0xFF);
        payload[cursor++] = (uint8_t)((data >> 8) & 0xFF);
        payload[cursor++] = (uint8_t)(data & 0xFF);
        return 1;
    } else {
        return -1;
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
    uint8_t cursorPos = cursor;
    cursor = 4;
    append(value);
    cursor = cursorPos;
}

int setData(uint8_t length, uint8_t* data) {
    cursor = 8;
    return append(length, data);
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

XBee-comms XBee-comms::begin(Stream &serial, int sleep_pin, int cts_pin) {
    sleepPin = sleep_pin;
    ctsPin = cts_pin;
    xbee = XBee();

    txStatusPayload = XBee-appPayload();
    txStatusPayload.setAppID(TELEMETRY_APP_ID);
    txStatusPayload.setMessageType(TX_STATUS_MSG);
    txStatusPayload.setVersion(TX_STATUS_VERSION);

    // Setup CTS pin if needed
    if (ctsPin > 0) {
        pinMode(cstPin INPUT);
    }

    // Make sure XBee is awake
    wake();

    xbee.begin(serial);
}


XBee-comms XBee-comms::begin(Stream &serial, int sleep_pin) {
    begin(serial, sleep_pin, -1);
}


XBee-comms XBee-comms::begin(Stream &serial) {
    begin(serial, -1, -1);
}


void XBee-comms::wake() {
    if (sleepPin >= 0) {
        pinMode(sleepPin, OUTPUT);
        digitalWrite (sleepPin LOW);
    }
}

void XBee-comms::sleep() {
    if (sleepPin >= 0) {
        pinMode(sleepPin, OUTPUT);
        digitalWrite (sleepPin HIGH);
    }
}

void XBee-comms::resetTxStatus() {
    TxStatus.millis = millis();
    // Tx Failures
    TxStatus.tx_packets = 0;
    TxStatus.tx_local_cts_timeout = 0;
    TxStatus.tx_local_timeout = 0;
    TxStatus.tx_cca_failure = 0;
    TxStatus.tx_invalid_dest = 0;
    TxStatus.tx_network_ack = 0;
    TxStatus.tx_not_joined = 0;
    TxStatus.tx_self_addr = 0;
    TxStatus.tx_addr_not_found = 0;
    TxStatus.tx_no_route = 0;
    TxStatus.tx_payload_too_big = 0;
    TxStatus.tx_other1 = 0;
    TxStatus.tx_other2 = 0;
}

void sendTxStatus() {
    now = millis();
    if (now > nextTxStatus) {
        TxStatus.millis = now;
        nextTxStatus = now + TX_STATUS_UPDATE_PERIOD;
        // check for rollover
        if( nextTxStatus < updateTime) {
            nextTxStatus = -1;
        }
        txStatusPayload.setData(TxStatus);
        transmit(txStatusPayload);
    }
}


int XBee-comms::transmit(XBee-payload data) {
    TxStatus.tx_packets++;
    // Make sure the XBee is awake
    wake();
    // Wait for CTS from the XBee or timeout
    bool gotCTS = false;
    if (ctsPin >= 0) {
        long timeout = millis() + TX_CTS_TIMEOUT;
        while(millis() < timeout) {
            if (digitalRead(ctsPin) == false) {
                gotCTS = true;
                break;
            } else {
                delay(10);
            }
        }
    } else {
        gotCTS = true;
    }

    if(gotCTS == true){
        // Send the Sensor data API frame to the Xbee
        xbee.getNextFrameId();
        
        xbee.send(zbTx);
        if(xbee.readPacket(TX_RESPONSE_TIMEOUT)){
            // We got a response from our local xbee
            if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE){
                // This is the "all OK" route...
                xbee.getResponse().getZBTxStatusResponse(txStatus);
                // Update failure counters if necessary
                switch(txStatus.getDeliveryStatus()){
                    case SUCCESS :
                    // Most likely route so get it out of the way first
                    // See whether we should send a TX Status report
                    if (millis() > nextTxUpdate) {

                        fillTxStatus();
                        xbee.getNextFrameId();
                        xbee.send(zbTxStatus);
                        if(xbee.readPacket(TX_RESPONSE_TIMEOUT)){
                            // We got a response from our local xbee
                            if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE){
                                // This is the "all OK" route...
                                xbee.getResponse().getZBTxStatusResponse(txStatus);
                                if(txStatus.getDeliveryStatus() == SUCCESS) {
                                    nextTxUpdate = millis() + TX_STATUS_UPDATE_PERIOD;
                                }
                            }
                        }
                    }
                    return 0;
                    break;
                    case CCA_FAILURE :
                    tx_cca_failure++;
                    return -5;
                    break;
                    case INVALID_DESTINATION_ENDPOINT_SUCCESS :
                    tx_invalid_dest++;
                    return -6;
                    break;
                    case NETWORK_ACK_FAILURE :
                    tx_network_ack++;
                    return -3;
                    break;
                    case NOT_JOINED_TO_NETWORK :
                    tx_not_joined++;
                    return -4;
                    break;
                    case SELF_ADDRESSED :
                    tx_self_addr++;
                    return -7;
                    break;
                    case ADDRESS_NOT_FOUND :
                    tx_addr_not_found++;
                    return -8;
                    break;
                    case ROUTE_NOT_FOUND :
                    tx_no_route++;
                    return -9;
                    break;
                    case PAYLOAD_TOO_LARGE :
                    tx_payload_too_big++;
                    return -10;
                    break;
                    default :
                    // Unexpected error
                    tx_other1++;
                    return -11;
                }
                return txStatus.getDeliveryStatus();
            } else {
                // There was another recieve packet or modem status???
                // Increment the failure counter
                tx_other2++;
                return -12;
            }
        } else {
            //Timed out waiting for a response from the XBee about the TX Status
            // Increment the failure counter
            tx_local_timeout++;
            return -2;
        }
    } else {
        // We didn't get CTS from the local XBee within the timeout
        // Update the failure counter
        tx_local_cts_timeout++;
        return -1;
    }
}
XBee-payload XBee-comms::receive();
