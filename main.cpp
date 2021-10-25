#include <iostream>
#include "Commport/Commport.h"
#include"Commport/CommportBoost.h"
#include <benchmark/benchmark.h>
#include "Commport/BoostSerial.h"

using namespace std;

void lowLevelTest(CCommPort &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.Write(command);
    port.Read(data,47);
}

void lowMultiThreadTest(CCommPort &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.ExecuteRW(command,data,47);
}

void BoostSerialPortThreadTest(BoostSerial &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.write(command);
    while(!port.available());
    data = port.readBytes(47);
}
void BoostSerialPortTest(CCommPortBoost &port,std::vector<unsigned char> &command,std::vector<unsigned char> &data)
{
    port.Execute(command,data,47);
}

static void BM_lowLevelTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPort cim("/dev/ttyUSB0");
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        lowLevelTest(cim,command,data);
    //
}

static void BM_BoostSerialPortThreadTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    BoostSerial cim;
    cim.open("/dev/ttyUSB0",921600);
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        BoostSerialPortThreadTest(cim,command,data);
    //
}
static void BM_BoostSerialPortTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPortBoost cim("/dev/ttyUSB0");
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        BoostSerialPortTest(cim,command,data);
    //
}

static void BM_lowMultiThreadTest(benchmark::State& state)
{
    //Этот код НЕ измеряется BENCHMARK
    CCommPort cim("/dev/ttyUSB0");
    std::vector<unsigned char> command = {0x02,0x03,0x06,0x37,0xfe,0xc7};
    std::vector<unsigned char> data;
    //
    //Этот код изменяется BENCHMARK
    for(auto _ : state)
        lowMultiThreadTest(cim,command,data);
    //
}

BENCHMARK(BM_BoostSerialPortTest);
BENCHMARK(BM_BoostSerialPortThreadTest);
BENCHMARK(BM_lowLevelTest);
BENCHMARK(BM_lowMultiThreadTest);
BENCHMARK_MAIN();
