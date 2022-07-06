#pragma once
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <chrono>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include "nn_topology.h"
#include "NetworkDriver.cpp"
#include "NeuralNetwork.h"


int main(int argc, char** argv)
{
    std::thread t1{ [&]() { NetworkDriver::getInstance().yearToDateSMA_MarketStackAPI("AMD"); std::cout << "\n"; } };
    t1.join();
    std::thread t2{ [&]() { NetworkDriver::getInstance().simpleMovingAverage_MarketStackAPI("AMD"); std::cout << "\n"; }};
    t2.join();



    std::cout<<"\nTOTAL API REQUESTS: "<<NetworkDriver::getInstance().getTotalRequest()<<std::endl;
    return 0;
}