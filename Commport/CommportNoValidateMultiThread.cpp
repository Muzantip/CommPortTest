#include "CommportNoValidateMultiThread.h""
#include <poll.h>
#include <unistd.h>
#include <functional>

//Если true значит данные были прочтены и читать не нужно
volatile bool g_isReadedNoVal = true;

CCommPortNoValidateMultiThread::CCommPortNoValidateMultiThread(const std::string &strPortName, const speed_t speed)
    : CCommPort(strPortName,speed)
    , m_readThread(std::bind(&CCommPortNoValidateMultiThread::_fnRead,this))
    , m_isReadErr{false}
    , m_readStop{false}
{

}

CCommPortNoValidateMultiThread::~CCommPortNoValidateMultiThread()
{
    if(m_readThread.joinable())
    {
        m_readStop = true;
        m_readThread.join();
        m_readStop = false;
    }
}

CCommPort::eResult CCommPortNoValidateMultiThread::Write(const std::vector<unsigned char> IN &data)
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

CCommPort::eResult CCommPortNoValidateMultiThread::Execute(const std::vector<unsigned char> IN &idata, std::vector<unsigned char> OUT &odata, const int IN readSize)
{
    eResult result = Write(idata);
    if(result == OK)
    {
        g_isReadedNoVal = false;
        m_readSize = readSize;
        for(;;)
        {
            std::unique_lock<std::mutex> lock(m_lobalMutex);
            m_condition.wait(lock , []{return g_isReadedNoVal;} );
            if(g_isReadedNoVal == true)
            {
                odata.assign(m_vreadData.begin(),m_vreadData.end());
                g_isReadedNoVal = false;
                if(m_isReadErr == true)
                    return READ_ERR;
                break;
            }
        }
    }
    else
    {
        return WRITE_ERR;
    }
    return OK;
}

void CCommPortNoValidateMultiThread::_fnRead()
{
    int tryCount = 0;// количество попыток, в случае таймаута
    for(;;)
    {
        if(m_readStop)
            break;
         if(g_isReadedNoVal)
             continue;

    if(m_vreadData.size() == 0)
        m_vreadData.resize(m_readSize);

    int tmpSize = 0;
    while(tmpSize < m_readSize)
    {
        size_t readSize = ::read(GetFD(),m_vreadData.data()+tmpSize,m_readSize);
        if(readSize == -1)
        {
            g_isReadedNoVal = true;
            m_isReadErr = true;
            m_condition.notify_one();
            break;
        }
        tmpSize+=readSize;
        m_readSize -= readSize;
    }
    if(tmpSize >= m_vreadData.size())
    {
        g_isReadedNoVal = true;
        m_isReadErr = false;
        m_condition.notify_one();
    }

    }
}
