#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <vector>
#include <list>
#include "dateAndTime.h"

class NetworkDriver
{
private:
	~NetworkDriver() = default;
	 NetworkDriver() = default;
	 NetworkDriver(const NetworkDriver&) = delete;

	calendar::dateAndTime DATE_ND;
	NetworkDriver& operator=(const NetworkDriver&) = delete;
	boost::posix_time::ptime Time = boost::posix_time::second_clock::local_time();
	boost::gregorian::date Date = Time.date();
	boost::gregorian::date yesterday = Time.date() - boost::gregorian::date_duration(1);
	std::string today_str = to_iso_extended_string(Date);
	std::string yesterday_str = to_iso_extended_string(yesterday);
	std::string marketstack_apikey() {
		return "ENTER_MARKETSTACK_API_KEY_HERE";
	}
	std::unordered_map<std::string, nlohmann::json> ytd_Map;//Map store Ticker Symbol as Key and Flattened JSON object is value.
	std::unordered_map<std::string, nlohmann::json> intraday_Map;
	std::unordered_map<std::string, nlohmann::json> intraday_range_Map;
	const std::unordered_map<std::string, std::string> fixed_map_interval_values = { 
		{"1min","1min"}, {"5min","5min"},{"10min","10min"},{"15min","15min"},{"30min","30min"},{"1hour","1hour"},{"3hour","3hour"},{"6hour","6hour"},{"12hour","12hour"},{"24hour","24hour"}
	};
	int ytd_Days = 0;
	int currentYear = (int)Date.year();
	int totalRequests = 0;
	bool callback = false;
public:
	static NetworkDriver& getInstance() {
		static NetworkDriver instance;
		return instance;
	};

	void getHistoricalDataMarketStackAPI(std::string ticker) {
		int track_days_apart = 0;
		int LimitParam = track_days_apart + 1;
		std::tuple<std::string, std::string> DATE_RANGE = DATE_ND.getDateRange();

		if (LimitParam > 1000) { LimitParam = 1000; } //1000 Due to constraint of API.
		cpr::Response r = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/eod" },
			cpr::Parameters{ {"access_key",marketstack_apikey()},{"symbols",ticker},
							 {"date_from",std::get<0>(DATE_RANGE)},{"date_to",std::get<1>(DATE_RANGE)},{"limit",std::to_string(LimitParam)} 
			});

		if (nlohmann::json::accept(r.text)) {
			try {
				using json = nlohmann::json;
				json jcomplete = json::parse(r.text);
				std::cout << std::setw(4) << jcomplete << "\n"; //RAW JSON Data
				auto v1 = jcomplete.flatten();
				std::cout << "\nv1 Flatten: \n" << std::setw(4) << v1 << "\n"; //Flattened JSON Data
				auto dataHigh = v1.find("/data/0/high");
				auto dataLow = v1.find("/data/0/low");
				double average = ((double)*dataHigh + (double)*dataLow) / 2;
				int days = (int)*v1.find("/pagination/total"); //Total days returned from API, weekends and holidays considered.
				for (int i = 1; i < days; i++) {
					dataHigh = v1.find("/data/" + std::to_string(i) + "/high");
					dataLow = v1.find("/data/" + std::to_string(i) + "/low");
					average += ((double)*dataHigh + (double)*dataLow) / 2;
				}
				average /= days;
				std::cout << "Weekly Average Price: " << average << "\n";
			}
			catch (std::exception E) {
				std::cout << "Historical Data Exception\n";
			}
		}

	}

	double yearToDateSMA_MarketStackAPI(std::string ticker) {
		boost::gregorian::date startingDate(currentYear, boost::gregorian::Jan, 1);
		boost::gregorian::date endingDate = Date;
		endingDate -= boost::gregorian::days(1); //End Date will be yesterday. It will not include the current day.
		std::string stDate_string = to_iso_extended_string(startingDate);
		std::string enDate_string = to_iso_extended_string(endingDate);
		int limitParam = abs((endingDate - startingDate).days());
		double ytd_sma = 0.0;

		cpr::Response r_ytd = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/eod" },
			cpr::Parameters{ {"access_key",marketstack_apikey()},{"symbols",ticker},
				{"date_from",stDate_string},{"date_to",enDate_string},{"limit",std::to_string(limitParam)} });
		totalRequests++;
		if (nlohmann::json::accept(r_ytd.text)) {
			try {
				using json = nlohmann::json;
				json jcomplete = json::parse(r_ytd.text);
				auto v2 = jcomplete.flatten();
				ytd_Map.try_emplace(ticker, v2);//C++17. Insert into Map ignore if already contained.
				int daysInData = (int)*v2.find("/pagination/total");
				double closingPricesAdded = 0;
				for (int i = 0; i < daysInData; i++) {
					closingPricesAdded += *v2.find("/data/" + std::to_string(i) + "/close");
				}
				ytd_sma = (closingPricesAdded / daysInData);
				std::cout << ticker << " YEAR TO DATE SIMPLE MOVING AVERAGE: " << (closingPricesAdded / daysInData) << "\n";
			}
			catch (std::exception E) {
				std::cout << ":::YTD SMA EXCEPTION:::" << "\n";
			}
		}
		if (callback == true) {
			callback = false;
			printSMA_rawData(ticker);
		}
		return ytd_sma;
	}

	double SMA_MarketStackAPI_V2(std::string ticker, calendar::dateAndTime& dateOBJ, std::tuple<std::string, std::string> DATE_RANGE) {
		std::vector<double> daily_prices;
		long limit = dateOBJ.daysApart(std::get<0>(DATE_RANGE), std::get<1>(DATE_RANGE));
		double closing_prices_added = 0;

		cpr::Response r_ytd = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/eod" },
			cpr::Parameters{ {"access_key",marketstack_apikey()},
							 {"symbols",ticker},
							 {"date_from",std::get<0>(DATE_RANGE)},
							 {"date_to",std::get<1>(DATE_RANGE)},
							 {"limit",std::to_string(limit)}
			});
		totalRequests++;
		json_response_handler(daily_prices,ytd_Map,r_ytd, ticker, "/close");
		for (double price : daily_prices) {
			closing_prices_added += price;
		}
		closing_prices_added /= daily_prices.size();
		std::cout << "SIMPLE MOVING AVERAGE : " << closing_prices_added << "\n";
		if (callback == true) {
			callback = false;
			printSMA_rawData(ticker);
		}
		return closing_prices_added;
	}

	/*TODO : Intraday range May cause an error if date range is far back enough such that the API has no instances to return. API keeps only the last 10k instances of data for each interval.*/
	std::vector<double> intraday_range(std::string ticker, std::string interval_input, std::tuple<std::string, std::string> DATE_RANGE) {
		std::vector<double> ird_range_vec;
		std::string interval = interval_input;
		if (fixed_map_interval_values.find(interval) == fixed_map_interval_values.end()) {
			std::cout << "interval not found, defualt of 1hour set\n";
			interval = "1hour";
		}

		cpr::Response r_intra = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/intraday"},
			cpr::Parameters{ {"access_key",marketstack_apikey()},
							 {"symbols",ticker},
							 {"limit","200"},
							 {"interval",interval},
							 {"date_from",std::get<0>(DATE_RANGE)},
							 {"date_to",std::get<1>(DATE_RANGE)},
							 {"sort","ASC"}
			});
		json_response_handler(ird_range_vec,intraday_range_Map,r_intra,ticker,"/last");
		return ird_range_vec;
	}

	void json_response_handler(std::vector<double>& ref_vec,std::unordered_map<std::string, nlohmann::json>& endpoint_Map, cpr::Response response, std::string ticker,std::string data_id) {
		if (nlohmann::json::accept(response.text)) {
			try {
				using json = nlohmann::json;
				json jcomplete = json::parse(response.text);
				auto v2 = jcomplete.flatten();
				endpoint_Map.try_emplace(ticker, v2);
				int num_intervals = (int)*v2.find("/pagination/total");
				double open = 0, close = 0;
				//std::cout << "TICKER: " << ticker << "\n============== = DATA============== = \n" << std::setw(4) << endpoint_Map.at(ticker) << "\n"; //Raw Data
				for (int i = 0; i < num_intervals; i++) {
					try {
						ref_vec.push_back((double)*v2.find("/data/" + std::to_string(i) + data_id));
					}
					catch (std::exception double_cast_type_error) {
						//intraday calls some values are 'null' in json. Intrday only contains the last 10,000 intervals from the current day.
					}
				}
			}
			catch (std::exception JSON_PARSE_ERROR) {
				std::cout << "EXCEPTION: json_response_handler :: JSON_PARSE_ERROR" << "\n";
				ref_vec.push_back(-1);
			}
		}
		else {
			std::cout << "json_response_handler:: INVALID JSON Received\n";
		}
	}

	/*Returns vector of the last known price of input stock. Inteval of 15 mins. */
	std::vector<double> intraday(std::string ticker) {
		std::vector<double> last_price_vec;
		cpr::Response response_intra = cpr::Get(cpr::Url{ "https://api.marketstack.com/v1/intraday" },
			cpr::Parameters{ {"access_key",marketstack_apikey()},
							 {"symbols",ticker},
							 {"limit","200"},
							 {"interval","15min"},
							 {"date_from",today_str},
							 {"sort","ASC"}
			});
		json_response_handler(last_price_vec, intraday_Map, response_intra, ticker, "/last" );
		return last_price_vec;
	}

	/*Prints the Year to Date data on a specific stock.
	If no such stock is contained in memory a call to the
	MarketStack Api is made to get Year to Date data. */
	void printSMA_rawData(std::string ticker) {
		if (ytd_Map.find(ticker) == ytd_Map.end()) {
			std::cout << "Ticker Symbol: '" << ticker << "' not found in memory. Requesting from API.\n";
			callback = true;
			yearToDateSMA_MarketStackAPI(ticker);
			return;
		}
		std::cout << "TICKER: " << ticker << "\n===============DATA===============\n" << std::setw(4) << ytd_Map.at(ticker) << "\n";
		return;
	}

	int getTotalRequest() {
		return totalRequests;
	}
};
