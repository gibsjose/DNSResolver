#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Record.hpp"

class ExtendedRecord : public Record {
public:
    ExtendedRecord(void) {
        ttl = 0;
        rdlength = 0;
        rdata = NULL;
    }

    ~ExtendedRecord(void)
    {
        if(rdata != NULL)
        {
            free(rdata);
        }
    }

    const uint32_t GetTTL(void) const { return ttl; }
    const unsigned short GetRecordDataLength(void) const { return rdlength; }
    const char * GetRecordData() const { return this->rdata; }

    void SetTTL(const uint32_t ttl) { this->ttl = ttl; }
    void SetRecordDataLength(const unsigned short rdlength) { this->rdlength = rdlength; }
    void SetRecordData(const char * aRdata, const unsigned short aLength);

    void Print(void);

    size_t Size(void);

    char * GetData(void);

    static std::string getIPFromBytes(unsigned char * aBytes, unsigned short aNumBytes);

protected:
    uint32_t ttl;
    unsigned short rdlength;
    char * rdata;
};

class AnswerRecord : public ExtendedRecord {};
class NameServerRecord : public ExtendedRecord {};
class AdditionalRecord : public ExtendedRecord {};
