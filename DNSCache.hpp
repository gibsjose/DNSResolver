#pragma once

#include <map>
#include "DNSPacket.h"

typedef std::map<std::string, DNSPacket *> PacketMap_t;
typedef std::map<std::string, StringNode> StringMap_t;

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
    StringNode(std::string & aIP, uint32_t aTTL)
    {
        mString = aIP;
        mTTL = aTTL;
    }

    std::string mString;
    time_t mTTL;
};

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
    //Store the raw DNS packet data keyed by the domain name
    PacketMap_t mPacketCache;
    StringMap_t mAddressCache;
    StringMap_t mAliasCache;
};
