#pragma once

#include <map>
#include "DNSPacket.h"

class DNSCacheElement
{
public:
    time_t mTTL;
    DNSPacket * mPacket;
};

typedef std::map<std::string, DNSCacheElement *> CacheMap_t;

class DNSCache
{
public:
    DNSCache();
    ~DNSCache();

    //Set the cached packet data for the given domain name
    void setDomain(std::string &, DNSPacket *);

    //Get the raw cached packet
    DNSPacket * getCachedDNSPacketByQuestionRawName(const std::string &) const;
private:
    //Store the raw DNS packet data keyed by the domain name
    CacheMap_t mCache;
};
