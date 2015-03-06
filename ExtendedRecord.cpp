#include "ExtendedRecord.hpp"

void ExtendedRecord::Print(void) {
    std::cout << "\t\tRaw Name --> " << rawName << std::endl;
    std::cout << "\t\tEncoded Name --> " << displayName << std::endl;
    std::cout << "\t\tType --> " << DecodeType(recordType) << std::endl;
    std::cout << "\t\tClass --> " << DecodeClass(recordClass) << std::endl;
    std::cout << "\t\tTTL --> " << ttl << std::endl;
    std::cout << "\t\tRecord Data Length --> " << rdlength << std::endl;
    std::cout << "\t\tRecord Data --> [data not printable]" /* << rdata */ << std::endl;
}

void ExtendedRecord::SetRecordData(const char * aRdata, const unsigned short aLength){
    // Both copy the rdata bytes and update the length.
    if(this->rdata != NULL)
    {
        free(rdata);
    }

    if(aRdata != NULL)
    {
        this->rdata = (char*)malloc(sizeof(char) * aLength);
        memcpy(this->rdata, aRdata, aLength);
        this->rdlength = aLength;
    }
    else
    {
        this->rdata = NULL;
        this->rdlength = 0;
    }
}

size_t ExtendedRecord::Size(void) {
    size_t size = 0;

    size += strlen(this->name) + 1;
    size += sizeof(this->recordType);
    size += sizeof(this->recordClass);
    size += sizeof(this->ttl);
    size += sizeof(this->rdlength);
    size += this->rdlength;

    return size;
}

char * ExtendedRecord::GetData(void) {
    //Malloc the required amount of space
    size_t dataLen = this->Size();
    data = (char *)malloc(dataLen);
    char * p = data;

    memcpy(p, this->name, strlen(this->name) + 1);
    p += strlen(this->name) + 1;

    unsigned short recordType = SWAP16(this->recordType);
    unsigned short recordClass = SWAP16(this->recordClass);
    uint32_t ttl = SWAP32(this->ttl);
    unsigned short rdlength = SWAP16(this->rdlength);

    memcpy(p, &(recordType), sizeof(recordType));
    p += sizeof(recordType);

    memcpy(p, &(recordClass), sizeof(recordClass));
    p += sizeof(recordClass);

    memcpy(p, &(ttl), sizeof(ttl));
    p += sizeof(ttl);

    memcpy(p, &(rdlength), sizeof(rdlength));
    p += sizeof(rdlength);

    if(this->rdlength > 0)
        memcpy(p, this->rdata, this->rdlength);

    return data;
}

std::string ExtendedRecord::getIPFromBytes(unsigned char * aBytes, unsigned short aNumBytes)
{
    char * str = (char *)malloc(sizeof(char) * 32);
    sprintf(str, "%u.%u.%u.%u", static_cast<unsigned>(aBytes[0]), static_cast<unsigned>(aBytes[1]), \
        static_cast<unsigned>(aBytes[2]), static_cast<unsigned>(aBytes[3]));
    std::string lStr(str);
    free(str);
    return lStr;
}
