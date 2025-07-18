// RF22ReliableDatagram.h
//
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// $Id: RF22ReliableDatagram.h,v 1.8 2014/04/01 05:06:44 mikem Exp mikem $

#ifndef RF22ReliableDatagram_h
#define RF22ReliableDatagram_h

#include "RF22Datagram.h"

// The acknowledgement bit in the FLAGS
#define RF22_FLAGS_ACK 0x80

/////////////////////////////////////////////////////////////////////
/// \class RF22ReliableDatagram RF22ReliableDatagram.h <RF22ReliableDatagram.h>
/// \brief RF22 subclass for sending addressed, acknowledged, retransmitted datagrams.
///
/// Extends RF22Datagram to define addressed, reliable datagrams with acknowledgement and retransmission.
/// Based on RF22Datagram, adds flags and sequence numbers. RF22ReliableDatagram is reliable in the sense
/// that messages are acknowledged, and unacknowledged messages are retransmitted until acknowledged or the
/// retries are exhausted.
/// When addressed messages are sent (by sendtoWait()), it will wait for an ack, and retransmit
/// after timeout until an ack is received or retries are exhausted.
/// When addressed messages are collected by the application (by recvfromAck()), 
/// an acknowledgement is automatically sent.
///
/// The retransmit timeout is randomly varied between timeout and timeout*2 to prevent collisions on all
/// retries when 2 nodes happen to start sending at the same time .
///
/// Each new message sent by sendtoWait() has its ID incremented.
///
/// An ack consists of a message with:
/// - TO set to the from address of the original message
/// - FROM set to this node address
/// - ID set to the ID of the original message
/// - FLAGS with the RF22_FLAGS_ACK bit set
///
/// Part of the Arduino RF22 library for operating with HopeRF RF22 compatible transceivers 
/// (see http://www.hoperf.com)
class RF22ReliableDatagram : public RF22Datagram
{
public:
    /// Constructor. 
    /// \param[in] thisAddress The address to assign to this node. Defaults to 0
    /// \param[in] slaveSelectPin the Arduino pin number of the output to use to select the RF22 before
    /// accessing it. Defaults to the normal SS pin for your Arduino (D10 for Diecimila, Uno etc, D53 for Mega)
    /// \param[in] interrupt The interrupt number to use. Default is interrupt 0 (Arduino input pin 2)
    RF22ReliableDatagram(uint8_t thisAddress = 0, uint8_t slaveSelectPin = SS, uint8_t interrupt = 0);

    /// Sets the minimum retransmit timeout. If sendtoWait is waiting for an ack 
    /// longer than this time (in milliseconds), 
    /// it will retransmit the message. Defaults to 200ms. The timeout is measured from the end of
    /// transmission of the message. It must be at least longer than the the transmit 
    /// time of the acknowledgement (preamble+6 octets) plus the latency/poll time of the receiver. 
    /// For fast modulation schemes you can considerably shorten this time.
    /// The actual timeout is randomly varied between timeout and timeout*2.
    /// \param[in] timeout The new timeout period in milliseconds
    void setTimeout(uint16_t timeout);

    /// Sets the max number of retries. Defaults to 3. If set to 0, the message will only be sent once.
    /// sendtoWait will give up and return false if there is no ack received after all transmissions time out.
    /// param[in] retries The maximum number a retries.
    void setRetries(uint8_t retries);

    /// Send the message and waits for an ack. Returns true if an acknowledgement is received.
    /// Synchronous: any message other than the desired ACK received while waiting is discarded.
    /// Blocks until an ACK is received or all retries are exhausted (ie up to retries*timeout milliseconds).
    /// \param[in] address The address to send the message to.
    /// \param[in] buf Pointer to the binary message to send
    /// \param[in] len Number of octets to send
    /// \return true if the message was transmitted and an acknowledgement was received.
    boolean sendtoWait(uint8_t* buf, uint8_t len, uint8_t address);

    /// If there is a valid message available for this node, send an acknowledgement to the SRC
    /// address (blocking until this is complete), then copy the message to buf and return true
    /// else return false. 
    /// If a message is copied, *len is set to the length..
    /// If from is not NULL, the SRC address is placed in *from.
    /// If to is not NULL, the DEST address is placed in *to.
    /// This is the preferred function for getting messages addressed to this node.
    /// If the message is not a broadcast, acknowledge to the sender before returning.
    /// You should be sure to call this function frequently enough to not miss any messages
    /// It is recommended that you call it in your main loop.
    /// \param[in] buf Location to copy the received message
    /// \param[in,out] len Available space in buf. Set to the actual number of octets copied.
    /// \param[in] from If present and not NULL, the referenced uint8_t will be set to the SRC address
    /// \param[in] to If present and not NULL, the referenced uint8_t will be set to the DEST address
    /// \param[in] id If present and not NULL, the referenced uint8_t will be set to the ID
    /// \param[in] flags If present and not NULL, the referenced uint8_t will be set to the FLAGS
    /// (not just those addressed to this node).
    /// \return true if a valid message was copied to buf
    boolean recvfromAck(uint8_t* buf, uint8_t* len, uint8_t* from = NULL, uint8_t* to = NULL, uint8_t* id = NULL, uint8_t* flags = NULL);

    /// Similar to recvfromAck(), this will block until either a valid message available for this node
    /// or the timeout expires. Starts the receiver automatically.
    /// You should be sure to call this function frequently enough to not miss any messages
    /// It is recommended that you call it in your main loop.
    /// \param[in] buf Location to copy the received message
    /// \param[in,out] len Available space in buf. Set to the actual number of octets copied.
    /// \param[in] timeout Maximum time to wait in milliseconds
    /// \param[in] from If present and not NULL, the referenced uint8_t will be set to the SRC address
    /// \param[in] to If present and not NULL, the referenced uint8_t will be set to the DEST address
    /// \param[in] id If present and not NULL, the referenced uint8_t will be set to the ID
    /// \param[in] flags If present and not NULL, the referenced uint8_t will be set to the FLAGS
    /// (not just those addressed to this node).
    /// \return true if a valid message was copied to buf
    boolean recvfromAckTimeout(uint8_t* buf, uint8_t* len,  uint16_t timeout, uint8_t* from = NULL, uint8_t* to = NULL, uint8_t* id = NULL, uint8_t* flags = NULL);

    /// Returns the number of retransmissions 
    /// we have had to send
    /// \return The number of retransmissions since initialisation.
    uint16_t retransmissions();

protected:
    /// Send an ACK for the message id to the given from address
    /// Blocks until the ACK has been sent
    void acknowledge(uint8_t id, uint8_t from);

    /// Checks whether the message currently in the Rx buffer is a new message, not previously received
    /// based on the from address and the sequence.  If it is new, it is acknowledged and returns true
    /// \return true if there is a message received and it is a new message
    boolean haveNewMessage();

private:
    /// Count of retransmissions we have had to send
    uint16_t _retransmissions;

    /// The last sequence number to be used
    /// Defaults to 0
    uint8_t _lastSequenceNumber;

    // Retransmit timeout (milliseconds)
    /// Defaults to 200
    uint16_t _timeout;

    // Retries (0 means one try only)
    /// Defaults to 3
    uint8_t _retries;

    /// Array of the last seen sequence number indexed by node address that sent it
    /// It is used for duplicate detection. Duplicated messages are re-acknowledged when received 
    /// (this is generally due to lost ACKs, causing the sender to retransmit, even though we have already
    /// received that message)
    uint8_t _seenIds[256];


};

#endif

