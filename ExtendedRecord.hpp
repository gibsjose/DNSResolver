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
        rdata.clear();
    }

    const uint32_t GetTTL(void) const { return ttl; }
    const unsigned short GetRecordDataLength(void) const { return rdlength; }
    const std::string & GetRecordData(void) const { return rdata; }

    void SetTTL(const uint32_t ttl) { this->ttl = ttl; }
    void SetRecordDataLength(const unsigned short rdlength) { this->rdlength = rdlength; }
    void SetRecordData(const std::string & rdata) { this->rdata = rdata; }

    void Print(void);

    size_t Size(void);

    char * GetData(void);

    static std::string getIPFromBytes(unsigned char * aBytes, unsigned short aNumBytes);

protected:
    uint32_t ttl;
    unsigned short rdlength;
    std::string rdata;
};

class AnswerRecord : public ExtendedRecord {};
class NameServerRecord : public ExtendedRecord {};
class AdditionalRecord : public ExtendedRecord {};
