#include <iostream>
#include "Commport/Commport.h"
#include"Commport/CommportBoost.h"
#include "Commport/CommportNoValidate.h"
#include "Commport/CommportMultiThread.h"
#include "Commport/CommportNoValidateMultiThread.h"
#include <benchmark/benchmark.h>
#include <ctime>

using namespace std;

const std::string g_strPortName = "/dev/ttyUSB0";
//std::vector<unsigned char> g_command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
const std::vector<unsigned char> g_command = {0x02,0x03,0x06,0x37,0xfe,0xc7,0x02,0x03,0x06,0x37,0xfe,0xc7,0x02,0x03,0x06,0x37,0xfe,0xc7,0x02,0x03,0x06,0x37,0xfe,0xc7
                                       ,0x02,0x03,0x06,0x37,0xfe,0xc7,0x02,0x03,0x06,0x37,0xfe,0xc7,0x02,0x03,0x06,0x37,0xfe,0xc7,0x02,0x03,0x06,0x37,0xfe};

void Test_Commport_Low(CCommPort &port,std::vector<unsigned char> &data)
{
    if(port.Write(g_command) != CCommPort::OK)
    {
        std::cout<<__FUNCTION__<<"() Write Err"<<std::endl;
    }
    if(port.Read(data,47) != CCommPort::OK)
    {
        std::cout<<__FUNCTION__<<"() Read Err"<<std::endl;
    }
}

void Test_Commport_Low_No_Validate(CCommPortNoValidate &port,std::vector<unsigned char> &data)
{
    if(port.Write(g_command) != CCommPort::OK)
    {
        std::cout<<__FUNCTION__<<"() Write Err"<<std::endl;
    }
    if(port.Read(data,47) != CCommPort::OK)
    {
        std::cout<<__FUNCTION__<<"() Read Err"<<std::endl;
    }
}

void Test_Commport_Low_Multi_Thread(CCommPortMultiThread &port,std::vector<unsigned char> &data)
{
    if(port.Execute(g_command,data,47) != CCommPort::OK)
    {
        std::cout<<__FUNCTION__<<"() Err"<<std::endl;
    }
}

void Test_Commport_Low_Multi_Thread_No_Validate(CCommPortNoValidateMultiThread &port,std::vector<unsigned char> &data)
{
    if(port.Execute(g_command,data,47) != CCommPort::OK)
    {
        std::cout<<__FUNCTION__<<"() Err"<<std::endl;
    }
}

void BoostSerialPortTest(CCommPortBoost &port,std::vector<unsigned char> &data)
{
    if(port.Execute(g_command,data,47) != CCommPortBoost::OK)
    {
        std::cout<<__FUNCTION__<<"() Err"<<std::endl;
    }
}

static void BM_Test_Commport_Low(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPort port(g_strPortName);
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        Test_Commport_Low(port,data);
    //
}

static void BM_Test_Commport_Low_No_Validate(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPortNoValidate port(g_strPortName);
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        Test_Commport_Low_No_Validate(port,data);
    //
}

static void BM_Test_Commport_Boost(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPortBoost port(g_strPortName);
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        BoostSerialPortTest(port,data);
    //
}

static void BM_Test_Commport_Low_Multi_Thread(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPortMultiThread port(g_strPortName);
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        Test_Commport_Low_Multi_Thread(port,data);
    //
}
static void BM_Test_Commport_Low_Multi_Thread_No_Validate(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPortNoValidateMultiThread port(g_strPortName);
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        Test_Commport_Low_Multi_Thread_No_Validate(port,data);
    //
}

BENCHMARK(BM_Test_Commport_Low);
BENCHMARK(BM_Test_Commport_Low_No_Validate);
BENCHMARK(BM_Test_Commport_Low_Multi_Thread);
BENCHMARK(BM_Test_Commport_Low_Multi_Thread_No_Validate);
BENCHMARK(BM_Test_Commport_Boost);
BENCHMARK_MAIN();
