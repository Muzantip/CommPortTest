#pragma once
#include <iostream>
#include <termios.h>
#include <vector>
#include <thread>
#define IN
#define OUT

class CCommPort
{
    enum eResult
    {
      OK,           //Удачно
      CONNECT_ERR,  //Ошибка при подключении к порту
      TERMIOS_ERR,  //Ошибка при настройке порта через termios
      FD_ERR,       //Ошибка актуальности файлового дискриптора
      WRITE_ERR,    //Ошибка при записи данных
      READ_ERR,     //Ошибка при чтении данных
      TIMEOUT_ERR,  //Время вышло
    };

public:
    CCommPort(const std::string IN &strPortName, const speed_t IN speed = B921600);
    ~CCommPort();

    eResult Write(const std::vector<unsigned char> IN &data);
    eResult Read(std::vector<unsigned char> OUT &data, int IN blockSize, const int IN timeout = 1000);
    void Close();

    //Пишем в одном потоке, читаем в другом потоке
    eResult ExecuteRW(const std::vector<unsigned char> IN &idata, std::vector<unsigned char> &odata, const int readSize);

private://функции
    eResult _connect(const std::string IN strPortName, const speed_t IN speed = B921600);
    eResult _fdIsValid();

    //Поток чтения данных
  //  std::thread m_readThread;
    void _fnRead();
    std::vector<unsigned char> m_vreadData;
    //Сколько байт надо читать
    int m_readSize;

private://Переменные

    //файловый дискриптор порта
    int m_fd;
};
