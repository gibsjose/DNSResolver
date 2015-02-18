#include "DNSCache.hpp"

DNSCache::DNSCache(){}

DNSCache::~DNSCache()
{
    //Clean up the cache by releasing memory from the packets.
    for(PacketMap_t::iterator lIter = mPacketCache.begin();
        lIter != mPacketCache.end();
        lIter++)
    {
        delete lIter.second;
        lIter.second = nullptr;
    }

    for(StringMap_t::iterator lIter = mAddressCache.begin();
        lIter != mAddressCache.end();
        lIter++)
    {
        delete lIter.second;
        lIter.second = nullptr;
    }

    for(StringMap_t::iterator lIter = mAliasCache.begin();
        lIter != mAliasCache.end();
        lIter++)
    {
        delete lIter.second;
        lIter.second = nullptr;
    }

    mPacketCache.clear();
    mAddressCache.clear();
    mAliasCache.clear();
}

//Set the cached packet data for the given domain name
void DNSCache::AddPacket(const std::string & aDomain, const DNSPacket & aPacket)
{
    // Create a copy of the packet.
    DNSPacket * lPacket = new DNSPacket(aPacket);

    //Find the minimum TTL in the answers, name servers and additional records.
    uint32_t lMinTTL;
    bool lGotFirst = false;

    for(std::vector<AnswerRecord>::iterator lIter = lPacket->GetAnswerSection().begin();
        lIter != lPacket->GetAnswerSection().end();
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

    for(std::vector<NameServerRecord>::iterator lIter = lPacket->GetNameServerSection().begin();
        lIter != lPacket->GetNameServerSection().end();
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

    for(std::vector<AdditionalRecord>::iterator lIter = lPacket->GetAdditionalSection().begin();
        lIter != lPacket->GetAdditionalSection().end();
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
    //Add the seconds to live to now.
    PacketNode * lPacketNode = new PacketNode(lPacket, time(NULL) + lMinTTL);

    //If the packet exists already, overwrite but do it after releasing memory.
    PacketMap_t::iterator lIter = mPacketCache.find(aDomain);
    if(lIter != mPacketCache.end())
    {
        delete lIter.second;
        lIter.second = nullptr;
        mPacketCache.erase(lIter);
    }

    //Put the cache element in the map.
    mCache[aDomain] = lPacketNode;
}

void DNSCache::AddAddress(const std::string & aName, const std::string & aIP, uint32_t aTTL)
{
    StringMap_t::iterator lIter = mAddressCache.find(aName);
    if(lIter != mAddressCache.end())
    {
        delete lIter.second;
        mAddressCache.erase(lIter);
    }
    mAddressCache[aName] = new StringNode(aIP, aTTL);
}

void DNSCache::AddAlias(const std::string & aAlias, const std::string & aName, uint32_t aTTL)
{
    StringMap_t::iterator lIter = mAliasCache.find(aAlias);
    if(lIter != mAliasCache.end())
    {
        delete lIter.second;
        mAliasCache.erase(lIter);
    }
    mAliasCache[aAlias] = new StringNode(aName, aTTL);
}


//Get the raw cached packet
DNSPacket * DNSCache::GetPacket(const std::string & aDomain) const
{
    PacketMap_t::iterator lIter = mPacketCache.find(aDomain);
    if(lIter == mPacketCache.end())
    {
        throw GeneralException("Item not in packet cache: " + aDomain);
    }
    else
    {
        //There is a candidate packet, but return it only if it has a TTL equal
        //to or greater than now.
        if(time(NULL) <= lIter.second.mTTL)
        {
            return lIter.second.mPacket;
        }
        else
        {
            throw TTLExpiredException("The TTL has expired for packet: " + aDomain);
        }
    }
}

std::string & DNSCache::GetAddress(const std::string & aName) const
{
    StringMap_t::iterator lIter = mAddressCache.find(aName);
    if(lIter == mAddressCache.end())
    {
        throw GeneralException("Item not in address cache: " + aName);
    }
    else
    {
        //There is a candidate address, but return it only if it has a TTL equal
        //to or greater than now.
        if(time(NULL) <= lIter.second.mTTL)
        {
            return lIter.second.mString;
        }
        else
        {
            throw TTLExpiredException("The TTL has expired for item: " + aName);
        }
    }
}

std::string & DNSCache::GetAlias(const std::string & aAlias) const
{
    StringMap_t::iterator lIter = mAliasCache.find(aAlias);
    if(lIter == mAliasCache.end())
    {
        throw GeneralException("Item not in alias cache: " + aAlias);
    }
    else
    {
        //There is a candidate alias, but return it only if it has a TTL equal
        //to or greater than now.
        if(time(NULL) <= lIter.second.mTTL)
        {
            return lIter.second.mString;
        }
        else
        {
            throw TTLExpiredException("The TTL has expired for item: " + aAlias);
        }
    }
}
