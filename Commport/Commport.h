#pragma once
#include <iostream>
#include <termios.h>
#include <vector>
#include <thread>
#define IN
#define OUT

class CCommPort
{
public:
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

    eResult Connect(const std::string IN strPortName, const speed_t IN speed = B921600);

    eResult Write(const std::vector<unsigned char> IN &data);
    eResult Read(std::vector<unsigned char> OUT &data, int IN blockSize, const int IN timeout = 1000);
    void Close();

    int GetFD() const {return m_fd;}


private://функции
    eResult _fdIsValid();


private://Переменные

    //файловый дискриптор порта
    int m_fd;
};
