#include <iostream>
#include "Commport/Commport.h"
#include"Commport/CommportBoost.h"
#include <benchmark/benchmark.h>

using namespace std;

const std::string g_strPortName = "/dev/ttyUSB2";

void lowLevelTest(CCommPort &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.Write(command);
    port.Read(data,47);
}

void lowMultiThreadTest(CCommPort &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.ExecuteRW(command,data,47);
}

void BoostSerialPortTest(CCommPortBoost &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.Execute(command,data,47);
}

static void BM_lowLevelTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPort port(g_strPortName);
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        lowLevelTest(port,command,data);
    //
}

static void BM_BoostSerialPortTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPortBoost port(g_strPortName);
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        BoostSerialPortTest(port,command,data);
    //
}

static void BM_lowMultiThreadTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPort port(g_strPortName);
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        lowMultiThreadTest(port,command,data);
    //
}

BENCHMARK(BM_BoostSerialPortTest);
BENCHMARK(BM_lowLevelTest);
BENCHMARK(BM_lowMultiThreadTest);
BENCHMARK_MAIN();
