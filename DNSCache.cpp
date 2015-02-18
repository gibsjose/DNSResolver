#include "DNSCache.hpp"

DNSCache::DNSCache()
{

}

DNSCache::~DNSCache()
{
    //Clean up the cache by releasing memory from the packets.
    for(CacheMap_t::iterator lIter = mCache.begin();
        lIter != mCache.end();
        lIter++)
    {
        delete mCache.second.mPacket;
        mCache.second.mPacket = nullptr;
    }
    mCache.clear();
}

//Set the cached packet data for the given domain name
void setDomain(std::string & aDomain, DNSPacket * aPacket)
{
    //Find the minimum TTL in the answers, name servers and additional records.
    uint32_t lMinTTL;
    bool lGotFirst = false;

    for(std::vector<AnswerRecord>::iterator lIter = aPacket->GetAnswerSection().begin();
        lIter != aPacket->GetAnswerSection().end();
        lIter++)
    {
        if(!lGotFirst)
        {
            //Grab an initial value if one has not yet been chosen.
            lMinTTL = lIter.GetTTL();
            lGotFirst = true;
        }
        else
        {
            //If the current TTL is less than the min, grab that one.
            if(lIter.GetTTL() < lMinTTL)
            {
                lMinTTL = liter.GetTTL();
            }
        }
    }

    for(std::vector<NameServerRecord>::iterator lIter = aPacket->GetNameServerSection().begin();
        lIter != aPacket->GetNameServerSection().end();
        lIter++)
    {
        if(!lGotFirst)
        {
            //Grab an initial value if one has not yet been chosen.
            lMinTTL = lIter.GetTTL();
            lGotFirst = true;
        }
        else
        {
            //If the current TTL is less than the min, grab that one.
            if(lIter.GetTTL() < lMinTTL)
            {
                lMinTTL = liter.GetTTL();
            }
        }
    }

    for(std::vector<AdditionalRecord>::iterator lIter = aPacket->GetAdditionalSection().begin();
        lIter != aPacket->GetAdditionalSection().end();
        lIter++)
    {
        if(!lGotFirst)
        {
            //Grab an initial value if one has not yet been chosen.
            lMinTTL = lIter.GetTTL();
            lGotFirst = true;
        }
        else
        {
            //If the current TTL is less than the min, grab that one.
            if(lIter.GetTTL() < lMinTTL)
            {
                lMinTTL = liter.GetTTL();
            }
        }
    }

    //Construct the cache element with the packet data and the minimum TTL.
    DNSCacheElement * lCacheElement = new DNSCacheElement();
    lCacheElement.mTTL = time(NULL) + lMinTTL;  //Add the seconds to live to now.
    lCacheElement.mPacket = aPacket;

    //Put the cache element in the map.
    mCache[aDomain] = lCacheElement;
}

//Get the raw cached packet
DNSPacket * getCachedDNSPacketByQuestionRawName(const std::string & aDomain) const
{
    Cache_t::iterator lIter = mCache.find(aDomain);
    if(lIter != mCache.end())
    {
        //There is a candidate packet, but return it only if it has a TTL equal
        //to or greater than now.
        if(time(NULL) <= lIter.second.GetTTL())
        {
            return lIter.second;
        }
    }
    else
    {
        return nullptr;
    }
}
