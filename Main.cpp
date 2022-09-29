#pragma once
#pragma comment(lib,"python310")
#define _USE_MATH_DEFINES
#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <tuple>
#include <thread>
#include <cmath>
#include <Eigen/Dense>
#include <bitset>
#include <set>
#include <cmath>
#include <Python.h>
#include <matplotlibcpp.h>
#include "customFunctions.h"
#include "dateAndTime.h"
#include "NetworkDriver.cpp"

namespace plt = matplotlibcpp;

void graph_data(std::string ticker, std::vector<double>& values, std::string x_label, std::string y_label) {
    plt::plot(values);
    plt::xlabel(x_label);
    plt::ylabel(y_label);
    plt::title(ticker);
    plt::show();
}
std::string getTicker() {
    std::cout << "Enter Ticker sysmbol:\n";
    std::string ticker = "";
    while (ticker.size() == 0 || ticker.size() > 5) {
        std::cin >> ticker;
        if (ticker.size() == 0 || ticker.size() > 5) {
            std::cout << "Invalid ticker, please enter again\n";
        }
        for (auto& a : ticker){ a = toupper(a);}
        for (char& c : ticker) {
            if (!isalpha(c)) {
                std::cout << "Not a valid character in input >"<<c<<"<\n";
                ticker = "";
                break;
            }
        }
    }
    return ticker;
}
const std::vector<std::string> stocks_121list = { "AAPL",
"MSFT",
"AMZN",
"TSLA",
"GOOGL",
"GOOG",
"UNH",
"JNJ",
"XOM",
"NVDA",
"JPM",
"PG",
"META",
"V",
"CVX",
"HD",
"MA",
"ABBV",
"PFE",
"LLY",
"PEP",
"BAC",
"KO",
"COST",
"MRK",
"TMO",
"AVGO",
"WMT",
"DIS",
"MCD",
"ABT",
"DHR",
"CSCO",
"ACN",
"VZ",
"NEE",
"WFC",
"PM",
"TXN",
"BMY",
"CRM",
"CMCSA",
"COP",
"QCOM",
"LIN",
"ADBE",
"UNP",
"CVS",
"UPS",
"NKE",
"RTX",
"LOW",
"AMD",
"AMGN",
"PARA",
"MAS",
"MGM",
"SBNY",
"RCL",
"FOXA",
"IPG",
"L",
"NRG",
"RE",
"AAP",
"CMA",
"NLSN",
"TFX",
"CPB",
"CE",
"GL",
"HAS",
"HSIC",
"CRL",
"CCL",
"PHM",
"EMN",
"TAP",
"HII",
"BIO",
"QRVO",
"WRK",
"CZR",
"MKTX",
"BBWI",
"CDAY",
"FFIV",
"ZION",
"JNPR",
"AAL",
"REG",
"TPR",
"BWA",
"RHI",
"PNW",
"ALLE",
"WHR",
"AIZ",
"ROL",
"LUMN",
"LNC",
"FBHS",
"IVZ",
"SEE",
"PNR",
"OGN",
"AOS",
"WYNN",
"XRAY",
"BEN",
"FRT",
"UHS",
"DXC",
"NWSA",
"NCLH",
"NWL",
"ALK",
"DVA",
"MHK",
"FOX",
"RL",
};
const std::vector<std::string> list_of_10_stocks = { "AAPL","BA","C","DAL","FDX","GOOG","H","IBM","JNJ","KO" };
std::vector<std::string> shared_memory_for_threads;


//Demo Functions Below

void intraday(std::string ticker, std::vector<double>& values){
    //get and chart intraday prices (15 minute intervals)
    values = NetworkDriver::getInstance().intraday(ticker);
    std::cout << "values size: " << values.size() << "\n";
    graph_data(ticker, values, "15 Min intervals", "Price");
}//TODO: Will not work on weekends

void intraday_range(std::string ticker, std::vector<double>& values) {
    std::string interval_x = "15min";     
    std::string y_label = "Price $";
    calendar::dateAndTime Date;
    auto date_range = Date.getDateRange();

    values=NetworkDriver::getInstance().intraday_range(ticker,interval_x, date_range); // last price
    graph_data(ticker, values, "15 Minute Intervals", y_label);
}

void SMA_single_stock(std::string ticker, calendar::dateAndTime Date, std::vector<double> sma_values) {
    std::tuple<std::string, std::string> DATE_RANGE = Date.getDateRange();
    double sma = NetworkDriver::getInstance().SMA_MarketStackAPI_V2(ticker, Date, DATE_RANGE);
    
}

void intraday_multistock(std::vector<std::string> stock_list,std::string interval_x) {
    std::vector<double> price;
    calendar::dateAndTime Date;
    auto date_range = Date.getDateRange();
    for (std::string stock : stock_list) {
        price=NetworkDriver::getInstance().intraday_range(stock, interval_x, date_range);
        plt::plot(price);
        plt::xlabel(interval_x);
        plt::ylabel("Price");
        plt::title("Stock List");
    }
    plt::show();
}

void sample_task() {
    std::vector<double> values;
    intraday_range(getTicker(),std::ref(values));
}

void sample_tasks_2() {
    intraday_multistock(list_of_10_stocks, "3hour");
}


int main() {
    sample_task();
    sample_tasks_2();

    return 0;
}