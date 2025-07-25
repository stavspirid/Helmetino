// RF22Mesh.cpp
//
// Define addressed datagram
// 
// Part of the Arduino RF22 library for operating with HopeRF RF22 compatible transceivers 
// (see http://www.hoperf.com)
// RF22Datagram will be received only by the addressed node or all nodes within range if the 
// to address is RF22_BROADCAST_ADDRESS
//
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// $Id: RF22Mesh.cpp,v 1.5 2014/04/01 05:06:44 mikem Exp mikem $

#include "RF22Mesh.h"
#include <SPI.h>

uint8_t RF22Mesh::_tmpMessage[RF22_ROUTER_MAX_MESSAGE_LEN];

////////////////////////////////////////////////////////////////////
// Constructors
RF22Mesh::RF22Mesh(uint8_t thisAddress, uint8_t slaveSelectPin, uint8_t interrupt) 
    : RF22Router(thisAddress, slaveSelectPin, interrupt)
{
}

////////////////////////////////////////////////////////////////////
// Public methods

////////////////////////////////////////////////////////////////////
// Discovers a route to the destination (if necessary), sends and 
// waits for delivery to the next hop (but not for delivery to the final destination)
uint8_t RF22Mesh::sendtoWait(uint8_t* buf, uint8_t len, uint8_t address)
{
    if (len > RF22_MESH_MAX_MESSAGE_LEN)
	return RF22_ROUTER_ERROR_INVALID_LENGTH;

    RoutingTableEntry* route = getRouteTo(address);
    if (!route && !doArp(address))
	return RF22_ROUTER_ERROR_NO_ROUTE;

    // Now have a route. Contruct an applicaiotn layer message and dend it via that route
    MeshApplicationMessage* a = (MeshApplicationMessage*)&_tmpMessage;
    a->header.msgType = RF22_MESH_MESSAGE_TYPE_APPLICATION;
    memcpy(a->data, buf, len);
    return RF22Router::sendtoWait(_tmpMessage, sizeof(RF22Mesh::MeshMessageHeader) + len, address);
}

////////////////////////////////////////////////////////////////////
boolean RF22Mesh::doArp(uint8_t address)
{
    // Need to discover a route
    // Broadcast a route discovery message with nothing in it
    MeshRouteDiscoveryMessage* p = (MeshRouteDiscoveryMessage*)&_tmpMessage;
    p->header.msgType = RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_REQUEST;
    p->destlen = 1; 
    p->dest = address; // Who we are looking for
    uint8_t error = RF22Router::sendtoWait((uint8_t*)p, sizeof(RF22Mesh::MeshMessageHeader) + 2, RF22_BROADCAST_ADDRESS);
    if (error !=  RF22_ROUTER_ERROR_NONE)
	return false;
    
    // Wait for a reply, which will be unicast back to us
    // It will contain the complete route to the destination
    uint8_t messageLen = sizeof(_tmpMessage);
    // FIXME: timeout should be configurable
    unsigned long starttime = millis();
    while ((millis() - starttime) < 4000)
    {
	if (RF22Router::recvfromAck(_tmpMessage, &messageLen))
	{
	    if (   messageLen > 1
		&& p->header.msgType == RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_RESPONSE)
	    {
		MeshRouteDiscoveryMessage* d = (MeshRouteDiscoveryMessage*)p;
		// Got a reply, now add the next hop to the dest to the routing table
		// The first hop taken is the first octet
		addRouteTo(address, headerFrom());
		return true;
	    }
	}
    }
    return false;
}

////////////////////////////////////////////////////////////////////
// Called by RF22Router::recvfromAck whenever a message goes past
void RF22Mesh::peekAtMessage(RoutedMessage* message, uint8_t messageLen)
{
    MeshMessageHeader* m = (MeshMessageHeader*)message->data;
    if (   messageLen > 1 
	&& m->msgType == RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_RESPONSE)
    {
	// This is a unicast RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_RESPONSE messages 
	// being routed back to the originator here. Want to scrape some routing data out of the response
	// We can find the routes to all the nodes between here and the responding node
	MeshRouteDiscoveryMessage* d = (MeshRouteDiscoveryMessage*)message->data;
	addRouteTo(d->dest, headerFrom());
	uint8_t numRoutes = messageLen - sizeof(RoutedMessageHeader) - sizeof(MeshMessageHeader) - 2;
	uint8_t i;
	// Find us in the list of nodes that were traversed to get to the responding node
	for (i = 0; i < numRoutes; i++)
	    if (d->route[i] == _thisAddress)
		break;
	i++;
	while (i++ < numRoutes)
	    addRouteTo(d->route[i], headerFrom());
    }
    else if (   messageLen > 1 
	     && m->msgType == RF22_MESH_MESSAGE_TYPE_ROUTE_FAILURE)
    {
	MeshRouteFailureMessage* d = (MeshRouteFailureMessage*)message->data;
	deleteRouteTo(d->dest);
    }
}

////////////////////////////////////////////////////////////////////
// This is called when a message is to be delivered to the next hop
uint8_t RF22Mesh::route(RoutedMessage* message, uint8_t messageLen)
{
    uint8_t from = headerFrom(); // Might get clobbered during call to superclass route()
    uint8_t ret = RF22Router::route(message, messageLen);
    if (   ret == RF22_ROUTER_ERROR_NO_ROUTE
	|| ret == RF22_ROUTER_ERROR_UNABLE_TO_DELIVER)
    {
	// Cant deliver to the next hop. Delete the route
	deleteRouteTo(message->header.dest);
	if (message->header.source != _thisAddress)
	{
	    // This is being proxied, so tell the originator about it
	    MeshRouteFailureMessage* p = (MeshRouteFailureMessage*)&_tmpMessage;
	    p->header.msgType = RF22_MESH_MESSAGE_TYPE_ROUTE_FAILURE;
	    p->dest = message->header.dest; // Who you were trying to deliver to
	    // Make sure there is a route back towards whoever sent the original message
	    addRouteTo(message->header.source, from);
	    ret = RF22Router::sendtoWait((uint8_t*)p, sizeof(RF22Mesh::MeshMessageHeader) + 1, message->header.source);
	}
    }
    return ret;
}

////////////////////////////////////////////////////////////////////
// Subclasses may want to override
boolean RF22Mesh::isPhysicalAddress(uint8_t* address, uint8_t addresslen)
{
    // Can only handle physical addresses 1 octet long, which is the physical node address
    return addresslen == 1 && address[0] == _thisAddress;
}

////////////////////////////////////////////////////////////////////
boolean RF22Mesh::recvfromAck(uint8_t* buf, uint8_t* len, uint8_t* source, uint8_t* dest, uint8_t* id, uint8_t* flags)
{     
    uint8_t tmpMessageLen = sizeof(_tmpMessage);
    uint8_t _source;
    uint8_t _dest;
    uint8_t _id;
    uint8_t _flags;
    if (RF22Router::recvfromAck(_tmpMessage, &tmpMessageLen, &_source, &_dest, &_id, &_flags))
    {
	MeshMessageHeader* p = (MeshMessageHeader*)&_tmpMessage;

	if (   tmpMessageLen >= 1 
	    && p->msgType == RF22_MESH_MESSAGE_TYPE_APPLICATION)
	{
	    MeshApplicationMessage* a = (MeshApplicationMessage*)p;
	    // Handle application layer messages, presumably for our caller
	    if (source) *source = _source;
	    if (dest)   *dest   = _dest;
	    if (id)     *id     = _id;
	    if (flags)  *flags  = _flags;
	    uint8_t msgLen = tmpMessageLen - sizeof(MeshMessageHeader);
	    if (*len > msgLen)
		*len = msgLen;
	    memcpy(buf, a->data, *len);
	    
	    return true;
	}
	else if (   _dest == RF22_BROADCAST_ADDRESS 
		 && tmpMessageLen > 1 
		 && p->msgType == RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_REQUEST)
	{
	    MeshRouteDiscoveryMessage* d = (MeshRouteDiscoveryMessage*)p;
	    // Handle Route discovery requests
	    // Message is an array of node addresses the route request has already passed through
	    // If it originally came from us, ignore it
	    if (_source == _thisAddress)
		return false;
	    
	    uint8_t numRoutes = tmpMessageLen - sizeof(MeshMessageHeader) - 2;
	    uint8_t i;
	    // Are we already mentioned?
	    for (i = 0; i < numRoutes; i++)
		if (d->route[i] == _thisAddress)
		    return false; // Already been through us. Discard
	    
	    // Hasnt been past us yet, record routes back to the earlier nodes
	    addRouteTo(_source, headerFrom()); // The originator
	    for (i = 0; i < numRoutes; i++)
		addRouteTo(d->route[i], headerFrom());
	    if (isPhysicalAddress(&d->dest, d->destlen))
	    {
		// This route discovery is for us. Unicast the whole route back to the originator
		// as a RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_RESPONSE
		// We are certain to have a route there, becuase we just got it
		d->header.msgType = RF22_MESH_MESSAGE_TYPE_ROUTE_DISCOVERY_RESPONSE;
		RF22Router::sendtoWait((uint8_t*)d, tmpMessageLen, _source);
	    }
	    else if (i < _max_hops)
	    {
		// Its for someone else, rebroadcast it, after adding ourselves to the list
		d->route[numRoutes] = _thisAddress;
		tmpMessageLen++;
		// Have to impersonate the source
		// REVISIT: if this fails what can we do?
		RF22Router::sendtoWait(_tmpMessage, tmpMessageLen, RF22_BROADCAST_ADDRESS, _source);
	    }
	}
    }
    return false;
}

////////////////////////////////////////////////////////////////////
boolean RF22Mesh::recvfromAckTimeout(uint8_t* buf, uint8_t* len, uint16_t timeout, uint8_t* from, uint8_t* to, uint8_t* id, uint8_t* flags)
{  
    unsigned long starttime = millis();
    while ((millis() - starttime) < timeout)
    {
	if (recvfromAck(buf, len, from, to, id, flags))
	    return true;
    }
    return false;
}



