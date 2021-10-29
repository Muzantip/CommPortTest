#pragma once
#include "Commport.h"

#define IN
#define OUT

class CCommPortNoValidate : public CCommPort
{
public:
    CCommPortNoValidate(const std::string IN &strPortName, const speed_t IN speed=B921600);
    ~CCommPortNoValidate();

    eResult Write(const std::vector<unsigned char> IN &data);
    eResult Read(std::vector<unsigned char> OUT &data, int IN blockSize);
};
