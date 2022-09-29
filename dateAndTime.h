#pragma once
#include <iostream>
#include <string>
#include <tuple>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>

namespace calendar {
	
	class dateAndTime {
	private:
		boost::posix_time::ptime Time = boost::posix_time::second_clock::local_time();
		boost::gregorian::date current_date = Time.date();
		boost::gregorian::date yesterday_date = Time.date() - boost::gregorian::date_duration(1);
		std::string today = to_iso_extended_string(current_date);
		std::string yesterday = to_iso_extended_string(yesterday_date);
		int current_year  = (int)current_date.year();
		int current_month = (int)current_date.month();
		int current_day = (int)current_date.day();

	public:
		calendar::dateAndTime(){	};

		int daysApart(std::string start_date, std::string end_date);


		bool isValidDate(std::string& input);

		std::tuple<std::string, std::string> getDateRange();

		friend void print(dateAndTime& dt);

	};

	

}