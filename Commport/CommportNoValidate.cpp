#include "CommportNoValidate.h"
#include <unistd.h>

CCommPortNoValidate::CCommPortNoValidate(const std::string IN &strPortName, const speed_t IN speed)
    : CCommPort(strPortName,speed)
{
}

CCommPortNoValidate::~CCommPortNoValidate()
{

}

CCommPort::eResult CCommPortNoValidate::Write(const std::vector<unsigned char> IN &data)
{
    if(data.size() == 0)
    {
        return WRITE_ERR;
    }

    size_t writeSize = write(GetFD(),data.data(),data.size());
    if(writeSize == 0 && writeSize < data.size())
    {
        return WRITE_ERR;
    }
    return OK;
}

CCommPort::eResult CCommPortNoValidate::Read(std::vector<unsigned char> OUT &data, IN int blockSize)
{
    if(blockSize <=0 )
        return READ_ERR;
    data.resize(blockSize);
    int tmpSize =0;
    for(;;)
    {
        size_t readSize = ::read(GetFD(),data.data()+tmpSize,blockSize);
        if(readSize == -1)
            return READ_ERR;
        tmpSize += readSize;
        blockSize -= readSize;
        if(tmpSize >= data.size())
            break;
    }

    return OK;
}
