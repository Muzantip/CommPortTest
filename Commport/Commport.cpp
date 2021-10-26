#include "Commport.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <mutex>
#include <functional>
#include <cstring>
#include <condition_variable>

volatile bool g_isReaded = false;
volatile bool g_isReadErr = false;
std::mutex g_mtx;

std::mutex g_lobalMutex;
std::condition_variable g_condition;

CCommPort::CCommPort(const std::string &strPortName, const speed_t IN speed)
    : m_fd(-1)
    , m_readSize(0)
    , m_readThread(std::bind(&CCommPort::_fnRead,this))
{
    if(_connect(strPortName) != OK)
    {
        throw "Failed to connect to port";
    }
}

CCommPort::~CCommPort()
{
    Close();
    if(m_readThread.joinable())
     {
         m_readThread.detach();
     }
}

CCommPort::eResult CCommPort::Write(const std::vector<unsigned char> IN &data)
{
    if(_fdIsValid() != OK)
    {
        return FD_ERR;
    }else
    {
        if(data.size() == 0)
        {
            return WRITE_ERR;
        }

        size_t writeSize = write(m_fd,data.data(),data.size());
        if(writeSize < data.size())
        {
            return WRITE_ERR;
        }
    }
    return OK;
}

CCommPort::eResult CCommPort::Read(std::vector<unsigned char> OUT &data, IN int blockSize, const int timeout)
{
    if(_fdIsValid() != OK)
    {
        return FD_ERR;
    }else
    {
        /*
         * POLLIN - событие(Есть данные для чтения)
        */
        struct pollfd fds;
        fds.fd=m_fd;
        fds.events = POLLIN;
        int result = poll(&fds, 1, timeout);
        if(result == 0)
            return TIMEOUT_ERR;
        if(result < 0)
            return READ_ERR;
        if(result > 0)
        {
            data.resize(blockSize);
            int tmpSize =0;
            for(;;)
            {
                size_t readSize = ::read(m_fd,data.data()+tmpSize,blockSize);
                if(readSize == -1)
                    return READ_ERR;
                tmpSize += readSize;
                blockSize -= readSize;
                if(tmpSize >= data.size())
                    break;
            }
        }
    }
    return OK;
}

void CCommPort::Close()
{
    if(_fdIsValid() == OK)
    {
        close(m_fd);
    }
}

CCommPort::eResult CCommPort::ExecuteRW(const std::vector<unsigned char> IN &idata, std::vector<unsigned char> OUT &odata, const int readSize)
{
    eResult result = Write(idata);
    if(result == OK)
    {
        g_isReaded = false;
        m_readSize = readSize;
        for(;;)
        {
            std::unique_lock<std::mutex> lock(g_lobalMutex);
            g_condition.wait(lock , []{return g_isReaded;} );
            if(g_isReaded == true)
            {
                odata.assign(m_vreadData.begin(),m_vreadData.end());
                g_isReaded = false;
                if(g_isReadErr == true)
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

void CCommPort::_fnRead()
{
    int tryCount = 0;// количество попыток, в случае таймаута
    for(;;)
    {
        if(g_isReaded)
            continue;

        std::lock_guard<std::mutex> lock(g_mtx);
        if(m_vreadData.size() == 0)
            m_vreadData.resize(m_readSize);

        /*
         * POLLIN - событие(Есть данные для чтения)
        */
        struct pollfd fds;
        fds.fd=m_fd;
        fds.events = POLLIN;
        int result = poll(&fds, 1, 100); //ожидаем 100 мс
        if(result <= 0)
        {
            if(tryCount > 3)
            {
              g_isReaded = true;
              g_isReadErr = true;
              g_condition.notify_one();
              tryCount = 0;
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
                    size_t readSize = ::read(m_fd,m_vreadData.data()+tmpSize,m_readSize);
                    if(readSize == -1)
                    {
                        g_isReaded = true;
                        g_isReadErr = true;
                        g_condition.notify_one();
                        break;
                    }
                    tmpSize+=readSize;
                    m_readSize -= readSize;
                }
                if(tmpSize >= m_vreadData.size())
                {
                    g_isReaded = true;
                    g_isReadErr = false;
                    g_condition.notify_one();
                }
             }
        }
    }
}

CCommPort::eResult CCommPort::_connect(const std::string IN strPortName, const speed_t IN speed)
{
    /*
     * O_RDWR     - в открытый файл strPortName можно читать и писать
     * O_NOCTTY   - если strPortName указывает на терминальное устройство, то оно не станет терминалом управления процесса
     * S_IRWXU    - пользователь (владелец файла) имеет права на чтение, запись и выполнение файла
    */
    m_fd = ::open(strPortName.data(), O_RDWR | O_NOCTTY,S_IRWXU);
    if(m_fd < 0)
    {
        return CONNECT_ERR;
    }else
    {
         termios config;
         //Получаем текущие настройки порта
         if( tcgetattr( m_fd, &config ) < 0 )
         {
             return TERMIOS_ERR;
         }
         config.c_iflag = 0;
         config.c_oflag = 0;
         config.c_lflag = 0;

         /*
          * CS8     - маска размера символов
          * CREAD   - включить прием
          * CLOCAL  - игнорировать управление линиями с помощью модема
          * VMIN    - минимальное количество символов для неканонического ввода
          * VTIME   - время ожидания в децисекундах для неканонического ввода(ожидаем 100мс при чтении)
        */
         config.c_cflag = speed | CS8 | CREAD | CLOCAL;
         config.c_cc[ VMIN ]  = 0;
         config.c_cc[ VTIME ] = 10;

         /*
          * TCSANOW - изменения начинают действовать немедленно
        */
         if(tcsetattr(m_fd,TCSANOW,&config)< 0)
         {
             return TERMIOS_ERR;
         }
         //Получаем текущие настройки порта
         if( tcgetattr( m_fd, &config ) < 0 )
         {
             return TERMIOS_ERR;
         }else
         {
             /*
              * Согласно документации, функция tcsetattr() сообщает об успешном завершении, если хотя бы одно из запрошенных изменений может быть успешно выполнено
              * По этому мы проверяем действительно ли изменили скоросить, потому что это важный показатель
            */
             if(config.c_ispeed != speed)
             {
                 return TERMIOS_ERR;
             }
         }
         /*
          * TCIOFLUSH -  срочный сброс принятых, но не прочитанных данных и записанных, но не переданных данных
         */
         if(tcflush(m_fd, TCIOFLUSH) != 0 )
         {
             return TERMIOS_ERR;
         }
    }
    return OK;
}

CCommPort::eResult CCommPort::_fdIsValid()
{
    /*
     * F_GETFL - получить права доступа к файлу и флаги состояния файла; arg игнорируется
     * EBADF   - неправильный описатель файла
    */
    if(fcntl(m_fd,F_GETFL) == -1 ||  errno == EBADF)
        return FD_ERR;
    return OK;
}
