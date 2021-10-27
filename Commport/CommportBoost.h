#pragma once
#include <boost/asio.hpp>
#include <vector>
#include <iostream>
#define IN
#define OUT

class CCommPortBoost
{
public:
    enum eResult
    {
      OK,           //Удачно
      CONNECT_ERR,  //Ошибка при подключении к порту
      WRITE_ERR,    //Ошибка при записи данных
      READ_ERR,     //Ошибка при чтении данных
      TIMEOUT_ERR,  //Время вышло
      ERR,          //Общяя ошибка
    };
  public:
    CCommPortBoost(const std::string &strPortName);
    ~CCommPortBoost();

    eResult Execute(const std::vector<unsigned char> IN &command, std::vector<unsigned char> OUT &result, const int IN resultSize, const int IN timeout=1000);

    std::vector<unsigned char> GetData();

private:

    boost::asio::io_service m_ioService;
    boost::asio::serial_port m_port;

    std::vector<unsigned char> m_data;

private:

    eResult _connect(const std::string IN strPortName, const int IN baud);
    void _disconnect();

    eResult _write(const std::vector<unsigned char> IN &command);
    void _writeHandler(const boost::system::error_code &err, std::size_t writeBytes);

    eResult _read();
    void _readHandler(const boost::system::error_code &err, std::size_t readBytes);

    //Запускаем отработку обработчиков со стороны boost
    //Возвращает количество отработанных обработчиков
    int Run(const int millisec = 1000);
};
