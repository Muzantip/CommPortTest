#pragma once
#include "CommportBoost.h"
#include <boost/bind/bind.hpp>

CCommPortBoost::CCommPortBoost(const std::string &strPortName)
    : m_port(m_ioService)
{
    if(_connect(strPortName,921600) != OK)
    {
        throw "Failed to connect to port";
    }
}

CCommPortBoost::~CCommPortBoost()
{
    _disconnect();
}

CCommPortBoost::eResult CCommPortBoost::Execute(const std::vector<unsigned char> IN &command, std::vector<unsigned char> &result, const int IN resultSize, const int IN timeout)
{
    if(m_port.is_open())
    {
        m_data.resize(resultSize);
        _write(command);
        //предпологаем, что Run должен отработать как минимум 2 обработчика, запись и чтение
        if(Run() < 2)
        {
            return ERR;
        }else
        {
            result = m_data;
        }

    }else
    {
        return ERR;
    }
    return OK;
}

std::vector<unsigned char> CCommPortBoost::GetData()
{
    return m_data;
}

CCommPortBoost::eResult CCommPortBoost::_connect(const std::string IN strPortName, const int IN baud)
{
    if(m_port.is_open())
    {
        return OK;
    }

    m_port.open(strPortName);
    if(m_port.is_open())
    {
        m_port.set_option(boost::asio::serial_port_base::baud_rate(baud));
        m_port.set_option(boost::asio::serial_port_base::character_size(8));
        m_port.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        m_port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        m_port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    }
    else
    {
        return CONNECT_ERR;
    }
    return OK;
}

void CCommPortBoost::_disconnect()
{
    if(m_port.is_open())
    {
        m_port.close();
    }
}

CCommPortBoost::eResult CCommPortBoost::_write(const std::vector<unsigned char> &command)
{
    if(command.size() == 0)
        return WRITE_ERR;
    if(m_port.is_open())
    {
        m_port.async_write_some(
                    boost::asio::buffer(command.data(),command.size()),
                    boost::bind(&CCommPortBoost::_writeHandler,this,
                         boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
    }else
    {
        return WRITE_ERR;
    }
    return OK;
}

void CCommPortBoost::_writeHandler(const boost::system::error_code &err, std::size_t writeBytes)
{
    if(writeBytes > 0)
    {
        _read();
    }
}

CCommPortBoost::eResult CCommPortBoost::_read()
{
    if(m_port.is_open())
    {
        boost::asio::async_read(m_port,
                    boost::asio::buffer(m_data,m_data.size()),
                    boost::bind(&CCommPortBoost::_readHandler,this,
                         boost::asio::placeholders::error,
                         boost::asio::placeholders::bytes_transferred));
    }else
    {
        return READ_ERR;
    }
    return OK;
}

void CCommPortBoost::_readHandler(const boost::system::error_code &err, std::size_t readBytes)
{

}

int CCommPortBoost::Run(const int millisec)
{
    m_ioService.restart();
    std::chrono::milliseconds ms{millisec};
    size_t sHendlerCount =  m_ioService.run_for(ms);
    return sHendlerCount;
}
