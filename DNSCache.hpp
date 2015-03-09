#pragma once

#include <map>
#include "DNSPacket.h"

class PacketNode
{
public:
    PacketNode(DNSPacket * aPacket, time_t aTTL)
    {
        mPacket = aPacket;
        mTTL = aTTL;
    }
    ~PacketNode() { delete mPacket; }

    DNSPacket * mPacket;
    time_t mTTL;
};

class StringNode
{
public:
    StringNode(const std::string & aIP, const uint32_t aTTL)
    {
        mString = aIP;
        mTTL = aTTL;
    }

    std::string mString;
    time_t mTTL;
};

typedef std::map<std::string, PacketNode *> PacketMap_t;
typedef std::map<std::string, StringNode *> StringMap_t;

class DNSCache
{
public:
    DNSCache();
    ~DNSCache();

    //Set the cached packet data for the given domain name
    void AddPacket(const std::string &, const DNSPacket &);
    void AddAddress(const std::string &, const std::string &, uint32_t);
    void AddAlias(const std::string &, const std::string &, uint32_t);

    //Get the raw cached packet
    DNSPacket * GetPacket(const std::string &) const;
    std::string & GetAddress(const std::string & ) const;
    std::string & GetAlias(const std::string & ) const;
private:
    uint32_t getMinTTLFromPacket(const DNSPacket &) const;

    //Store the raw DNS packet data keyed by the domain name
    PacketMap_t mPacketCache;
    StringMap_t mAddressCache;
    StringMap_t mAliasCache;
};
