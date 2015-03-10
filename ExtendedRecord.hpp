#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Record.hpp"

class ExtendedRecord : public Record {
public:
    ExtendedRecord(void) : Record(){
        this->ttl = 0;
        this->rdlength = 0;
        this->rdata = nullptr;
    }

    // ~ExtendedRecord(void)
    // {
    //     if(this->rdata != nullptr)
    //     {
    //         // printf("~ExtendedRecord: Freeing this->rdata: %p\n", this->rdata);
    //         // delete this->rdata;
    //         this->rdata = nullptr;
    //         // printf("~ExtendedRecord: this->rdata: %p\n", this->rdata);
    //     }
    // }

    const uint32_t GetTTL(void) const { return ttl; }
    const unsigned short GetRecordDataLength(void) const { return rdlength; }
    const char * GetRecordData() const { return this->rdata; }

    void SetTTL(const uint32_t ttl) { this->ttl = ttl; }
    void SetRecordDataLength(const unsigned short rdlength) { this->rdlength = rdlength; }
    void SetRecordData(const char * aRdata, const unsigned short aLength);

    void Print(void);

    size_t Size(void);

    char * GetData(void);

    static std::string getIPFromBytes(const char * aBytes, const unsigned short aNumBytes);

protected:
    uint32_t ttl;
    unsigned short rdlength;
    char * rdata;
};

class AnswerRecord : public ExtendedRecord {};
class NameServerRecord : public ExtendedRecord {};
class AdditionalRecord : public ExtendedRecord {};
