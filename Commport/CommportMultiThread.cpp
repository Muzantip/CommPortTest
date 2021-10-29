#include "CommportMultiThread.h"
#include <poll.h>
#include <unistd.h>
#include <functional>

//Если true значит данные были прочтены и читать не нужно
volatile bool g_isReadedVal = true;

CCommPortMultiThread::CCommPortMultiThread(const std::string &strPortName, const speed_t speed)
    : CCommPort(strPortName,speed)
    , m_readThread(std::bind(&CCommPortMultiThread::_fnRead,this))
    , m_isReadErr{false}
    , m_readStop{false}
{

}

CCommPortMultiThread::~CCommPortMultiThread()
{
    if(m_readThread.joinable())
    {
        m_readStop = true;
        m_readThread.join();
        m_readStop = false;
    }
}

CCommPort::eResult CCommPortMultiThread::Execute(const std::vector<unsigned char> IN &idata, std::vector<unsigned char> OUT &odata, const int IN readSize)
{
    eResult result = Write(idata);
    if(result == OK)
    {
        g_isReadedVal = false;
        m_readSize = readSize;
        for(;;)
        {
            std::unique_lock<std::mutex> lock(m_lobalMutex);
            m_condition.wait(lock , []{return g_isReadedVal;} );
            if(g_isReadedVal == true)
            {
                odata.assign(m_vreadData.begin(),m_vreadData.end());
                g_isReadedVal = false;
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

void CCommPortMultiThread::_fnRead()
{
    int tryCount = 0;// количество попыток, в случае таймаута
    for(;;)
    {
        if(m_readStop)
            break;
         if(g_isReadedVal)
             continue;

        if(m_vreadData.size() == 0)
            m_vreadData.resize(m_readSize);

        /*
         * POLLIN - событие(Есть данные для чтения)
        */
        struct pollfd fds;
        fds.fd=GetFD();
        fds.events = POLLIN;
        int result = poll(&fds, 1, 100); //ожидаем 100 мс
        if(result <= 0)
        {
            if(tryCount > 3)
            {
              g_isReadedVal = true;
              m_isReadErr = true;
              tryCount = 0;
              m_condition.notify_one();
            }else
                tryCount++;
            continue;
        }
        if(result > 0)
        {
            if(fds.revents & POLLIN)
             {
                int tmpSize = 0;
                while(tmpSize < m_readSize)
                {
                    size_t readSize = ::read(GetFD(),m_vreadData.data()+tmpSize,m_readSize);
                    if(readSize == -1)
                    {
                        g_isReadedVal = true;
                        m_isReadErr = true;
                        m_condition.notify_one();
                        break;
                    }
                    tmpSize+=readSize;
                    m_readSize -= readSize;
                }
                if(tmpSize >= m_vreadData.size())
                {
                    g_isReadedVal = true;
                    m_isReadErr = false;
                    m_condition.notify_one();
                }
             }
        }
    }
}
