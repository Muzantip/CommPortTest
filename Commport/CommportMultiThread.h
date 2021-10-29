#include "Commport.h"
#include <thread>
#include <condition_variable>

#define IN
#define OUT

class CCommPortMultiThread : public CCommPort
{
  public:
    CCommPortMultiThread(const std::string IN &strPortName, const speed_t IN speed=B921600);
    ~CCommPortMultiThread();

    //Пишем в одном потоке, читаем в другом потоке
    eResult Execute(const std::vector<unsigned char> IN &idata, std::vector<unsigned char> OUT &odata, const int IN readSize);

private:

    //Поток чтения данных
    std::thread m_readThread;
    void _fnRead();
    std::vector<unsigned char> m_vreadData;
    //Сколько байт надо читать
    int m_readSize;


    //Возникла ошибка при чтении данных
    volatile bool m_isReadErr;
    //Останавливаем чтение
    volatile bool m_readStop;

    //Для остановки основного потока, которы ждет пока данные будут прочтены
    std::mutex m_lobalMutex;
    std::condition_variable m_condition;
    //
};
