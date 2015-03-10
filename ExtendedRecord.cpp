#include "ExtendedRecord.hpp"

void ExtendedRecord::Print(void) {
    std::cout << "\t\tRaw Name --> " << rawName << std::endl;
    std::cout << "\t\tEncoded Name --> " << displayName << std::endl;
    std::cout << "\t\tType --> " << DecodeType(recordType) << std::endl;
    std::cout << "\t\tClass --> " << DecodeClass(recordClass) << std::endl;
    std::cout << "\t\tTTL --> " << ttl << std::endl;
    std::cout << "\t\tRecord Data Length --> " << rdlength << std::endl;
    std::cout << "\t\tRecord Data --> ";

    if(rdata != nullptr)
    {
        if(recordType == TYPE_A)
        {
            std::cout << ExtendedRecord::getIPFromBytes(rdata, rdlength);
        }
        else if(recordType == TYPE_CNAME)
        {
            char * tempData = rdata;
            std::cout << Record::DecodeString(tempData, const_cast<const char **>(&tempData));
        }
        else
        {
            std::cout << "[Unprintable]";
        }
    }
    else
    {
        std::cout << "(null)";
    }
    std::cout << std::endl;
}

void ExtendedRecord::SetRecordData(const char * aRdata, const unsigned short aLength){
    // Both copy the rdata bytes and update the length.
    if(this->rdata != nullptr)
    {
        delete this->rdata;
        this->rdata = nullptr;
    }

    if(aRdata != nullptr)
    {
        this->rdata = (char*)malloc(sizeof(char) * aLength);
        memcpy(this->rdata, aRdata, aLength);
        this->rdlength = aLength;
    }
    else
    {
        this->rdata = nullptr;
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

    if(this->rdlength > 0 && this->rdata != nullptr)
    {
        memcpy(p, this->rdata, this->rdlength);
    }
    else
    {
        std::cout << "Skipping memcpy() for ExtendedRecord::GetData()." << std::endl;
    }

    return data;
}

std::string ExtendedRecord::getIPFromBytes(const char * aBytes, const unsigned short aNumBytes)
{
    if(aNumBytes != 4)
    {
        return std::string("ERROR");
    }
    else
    {
        char * str = (char *)malloc(sizeof(char) * 32);
        str[0] = '\0';

        sprintf(str,
                "%hu.%hu.%hu.%hu",
                static_cast<unsigned char>(aBytes[0]),
                static_cast<unsigned char>(aBytes[1]),
                static_cast<unsigned char>(aBytes[2]),
                static_cast<unsigned char>(aBytes[3])
        );
        std::string lStr(str);
        free(str);
        return lStr;
    }
}
