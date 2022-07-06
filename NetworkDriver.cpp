#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <vector>
#include <list>

class NetworkDriver
{
private:
	NetworkDriver() = default;
	~NetworkDriver() = default;
	NetworkDriver(const NetworkDriver&) = delete;
	NetworkDriver& operator=(const NetworkDriver&) = delete;
	std::string marketstack_apikey() {
		return "ENTER YOUR MARKETSTACK API KEY HERE";
	}
	boost::posix_time::ptime Time = boost::posix_time::second_clock::local_time();
	boost::gregorian::date Date = Time.date();
	int currentYear = (int)Date.year();
	std::unordered_map<std::string, nlohmann::json> ytdMap;//Ticker Symbol is Key , Flattened JSON object is value.
	int ytdDays = 0;
	bool callback = false;
	int totalRequests = 0;
public:
	static NetworkDriver& getInstance() {
		static NetworkDriver instance;
		return instance;
	};
	
	/*getUserDateMSA
	requests user input via console to enter year, month, and day in that order.
	Once valid input is recieved the date is appended to the passed by reference vector of type int.
	The valid date is formatted as a string and returned. API limitations of 10 years of historical data is taken into account.
	*/
	std::string getUserDateMSA(std::vector<int> &writeDateHere) {
		int numDaysInMonth[] = { -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		int day=0, month=0, year=0;
		std::string s_day,s_month,s_year;
		bool invalidDate = false;
		boost::gregorian::date farthestDateBack = Date - boost::gregorian::days(3652); //Given an arbitrary 10 year time frame the minimum number of leap years seen is 2 or a maximum of 3. 
		do {
			if (invalidDate == true) {invalidDate = false;}
			std::cout << "Enter Year as a 4 digit number from '2013' onwards.\n";
			std::cin >> year;
			std::cout << "Enter Month as a number 1-12\nJan[1], Feb[2], Mar[3], Apr[4], May[5], Jun[6], Jul[7], Aug[8], Sep[9], Oct[10], Nov[11], Dec[12]\n";
			std::cin >> month;
			std::cout << "Enter Day as a number 1-31\n";
			std::cin >> day;
			if (year % 4 == 0) { numDaysInMonth[2]++;} //In case of Leap Year
			if (day > numDaysInMonth[month] ||day < 1|| year<2013 || year>currentYear) {
				invalidDate = true; 
				std::cout << "Invlaid Date.\n";
			}
		} while (invalidDate == true);
		s_day=std::to_string(day);
		s_month=std::to_string(month);
		s_year = std::to_string(year);
		if (day < 10) {
			s_day = "0" + std::to_string(day);
		}
		if (month < 10) {
			s_month = "0" + std::to_string(month);
		}
		writeDateHere.at(0) = year;
		writeDateHere.at(1) = month;
		writeDateHere.at(2) = day;
		return s_year+"-"+s_month+"-"+s_day;
	}

	/*
	Makes two calls to the getUserDateMSA function to recieve Two dates formatted in string values.
	Stores returned strings from user input in a vector with an initial size of 2.
	Takes in an int value by reference to keep track of the days apart.
	*/
	std::vector<std::string> getUserDateRange(int& trackDaysApart) {
		std::vector<std::string> result(2); 
		std::vector<int> startDateVec(3), endDateVec(3);
		int Year = 0, Month = 1, Day = 2;
		std::cout << "*****Enter a START Date.*****\n";
		std::string date_from = getUserDateMSA(startDateVec);
		std::cout << "*****Enter an END Date.*****\n";
		std::string date_end = getUserDateMSA(endDateVec);
		result.at(0) = date_from;
		result.at(1) = date_end;
		boost::gregorian::date d_start(startDateVec.at(Year), startDateVec.at(Month), startDateVec.at(Day));
		boost::gregorian::date d_end(endDateVec.at(Year), endDateVec.at(Month), endDateVec.at(Day));
		if ((d_end - d_start).days() < 0) {
			std::string swap = result.at(0);
			result.at(0) = result.at(1);
			result.at(1) = swap;
		}
		trackDaysApart = abs((d_end - d_start).days());
		return result;
	}

	/*getHistoricalDataMarketStackAPI
	Calls Helper functions that Request user input for a start and end date range to
	be set as parameters for https get request for historical 
	stock market data (Average Price based on high and low of the days) of the given ticker symbol.
	startAndEndDates.at(0) = Start Date
	startAndEndDates.at(1) = End Date
 	*/
	void getHistoricalDataMarketStackAPI(std::string ticker) {
		int track_days_apart = 0;
		std::vector<std::string>startAndEndDates=getUserDateRange(track_days_apart);
		int LimitParam = track_days_apart + 1;
		if (LimitParam > 1000) { LimitParam = 1000; } //1000 Due to constraint of API.
		std::cout << "PRE GET REQ: DAYS APART TEST:\nDays Aprt: "<< track_days_apart <<"\n"<<"Start Date:"<< track_days_apart <<"\nEnd Date"<< track_days_apart <<"\n"<<"Limit Param: "<<LimitParam<<"\n";
		cpr::Response r = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/eod"},
		cpr::Parameters{ {"access_key",marketstack_apikey()},{"symbols",ticker},
						 {"date_from",startAndEndDates.at(0)},{"date_to",startAndEndDates.at(1)},{"limit",std::to_string(LimitParam)}});
		
		if (nlohmann::json::accept(r.text)) {
			try {
				using json = nlohmann::json;
				json jcomplete = json::parse(r.text);
				std::cout << std::setw(4) << jcomplete << "\n"; //RAW JSON Data
				auto v1 = jcomplete.flatten();
				std::cout << "\nv1 Flatten: \n" <<std::setw(4)<<v1<< "\n"; //Flattened JSON Data
				auto dataHigh = v1.find("/data/0/high");
				auto dataLow = v1.find("/data/0/low");
				double average=((double)*dataHigh + (double)*dataLow)/2;
				int days = (int)*v1.find("/pagination/total"); //Total days returned from API, weekends and holidays considered.
				for (int i = 1; i < days; i++) {
					dataHigh = v1.find("/data/"+std::to_string(i)+"/high");
					dataLow = v1.find("/data/"+std::to_string(i) +"/low");
					average += ((double)*dataHigh + (double)*dataLow) / 2;  
				}
				average /= days;
				std::cout << "Weekly Average Price: " << average<<"\n";
			}
			catch (std::exception E) {
				std::cout << "Caught Some exception E\n";
			}
		}
		
	}

	

	void yearToDateSMA_MarketStackAPI(std::string ticker) {
			boost::gregorian::date startingDate(currentYear, boost::gregorian::Jan, 1);
			boost::gregorian::date endingDate = Date;
			endingDate -= boost::gregorian::days(1); //End Date will be yesterday. It will not include the current day.
			std::string stDate_string = to_iso_extended_string(startingDate);
			std::string enDate_string = to_iso_extended_string(endingDate);
			int limitParam = abs((endingDate - startingDate).days());

			cpr::Response r_ytd = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/eod" },
			cpr::Parameters{ {"access_key",marketstack_apikey()},{"symbols",ticker},
				{"date_from",stDate_string},{"date_to",enDate_string},{"limit",std::to_string(limitParam)} });
			totalRequests++;
			if (nlohmann::json::accept(r_ytd.text)) {
				try {
					using json = nlohmann::json;
					json jcomplete = json::parse(r_ytd.text);
					auto v2 = jcomplete.flatten();
					ytdMap.try_emplace(ticker, v2);//C++17. Insert into Map ignore if already contained.
					int daysInData = (int)*v2.find("/pagination/total");
					double closingPricesAdded=0;
					for (int i = 0; i < daysInData; i++) {
						closingPricesAdded += *v2.find("/data/" + std::to_string(i) + "/close");
					}
					std::cout <<ticker<<" YEAR TO DATE SIMPLE MOVING AVERAGE: " << (closingPricesAdded / daysInData) << "\n";
				}
				catch (std::exception E) {
					std::cout << ":::YTD SMA EXCEPTION:::" << "\n";
				}
			}
			if (callback == true) {
				callback = false;
				printSMA_rawData(ticker);
			}
	}
	void simpleMovingAverage_MarketStackAPI(std::string ticker) {
		int daysApart=0;
		std::vector<std::string>dateRange=getUserDateRange(daysApart);//Handles Both Start and End Date
		boost::gregorian::date endingDate(boost::gregorian::from_string(dateRange.at(1)));
		boost::gregorian::date startingDate(boost::gregorian::from_string(dateRange.at(0)));
		std::string stDate_string = to_iso_extended_string(startingDate);//converting boost date for parameters in request.
		std::string enDate_string = to_iso_extended_string(endingDate);
		int limitParam = abs((endingDate - startingDate).days());
		

		cpr::Response r_ytd = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/eod" },
		cpr::Parameters{ {"access_key",marketstack_apikey()},
						 {"symbols",ticker},
						 {"date_from",stDate_string},
						 {"date_to",enDate_string},
						 {"limit",std::to_string(limitParam)}
		});
		totalRequests++;
		if (nlohmann::json::accept(r_ytd.text)) {
			try {
				using json = nlohmann::json;
				json jcomplete = json::parse(r_ytd.text);
				auto v2 = jcomplete.flatten();
				ytdMap.try_emplace(ticker, v2);//C++17. Insert into Map ignore if already contained.
				int daysInData = (int)*v2.find("/pagination/total");
				double closingPricesAdded = 0;
				for (int i = 0; i < daysInData; i++) {
					closingPricesAdded += *v2.find("/data/" + std::to_string(i) + "/close");
				}
				std::cout <<"SIMPLE MOVING AVERAGE : " << (closingPricesAdded / daysInData) << "\n";
			}
			catch (std::exception E) {
				std::cout << ":::EXCEPTION CAUGHT:::" << "\n";
			}
		}
		if (callback == true) {
			callback = false;
			printSMA_rawData(ticker);
		}
	}





	/*Prints the Year to Date data on a specific stock.
	If no such stock is contained in memory a call to the
	MarketStack Api is made to get Year to Date data. */
	void printSMA_rawData(std::string ticker) {
		if (ytdMap.find(ticker) == ytdMap.end()) {
			std::cout << "Ticker Symbol: '" << ticker << "' not found in memory. Requesting from API.\n";
			callback = true;
			yearToDateSMA_MarketStackAPI(ticker);
			return;
		}
		std::cout<<"TICKER: "<<ticker<<"\n===============DATA===============\n"<< std::setw(4) << ytdMap.at(ticker) << "\n";
		return;
	}



	int getTotalRequest() {
		return totalRequests;
	}
};
