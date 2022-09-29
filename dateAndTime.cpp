#include "dateAndTime.h"


std::tuple<std::string, std::string> calendar::dateAndTime::getDateRange() //TODO Add quick acess for 90, 100,180, 200 day 1Y 3Y sma
{	
	bool validDate = true;
	std::string user_start_date = "", user_end_date = "";
	std::string user_retype_dates = "0";

	std::cout << "Enter Start Date As: Month/Day/Year\n";
	do {
		do {
			do {
				std::cin >> user_start_date;
				validDate = isValidDate(user_start_date);
				if (validDate == false) { std::cout << "Invalid Start Date\n"; }
			} while (validDate == false);
			std::cout << "Enter an End Date As: Month/Day/Year\n";
			do {
				std::cin >> user_end_date;
				validDate = isValidDate(user_end_date);
				if (validDate == false) { std::cout << "Invalid End Date\n"; }
			} while (validDate == false);
			boost::gregorian::date start(boost::gregorian::from_string(user_start_date));
			boost::gregorian::date end(boost::gregorian::from_string(user_end_date));
			if (start > end) {
				std::cout << "!Error Start Date is After the End Date!\n";
				validDate = false;
			}
			else { validDate = true; }
		} while (validDate == false);
		user_retype_dates = "0";
		std::cout << "Confirm entered Dates, press any letter.\nTo retype dates press '1' and hit enter.\n";
		std::cin >> user_retype_dates;
	} while (user_retype_dates == "1");
	return std::make_tuple(user_start_date,user_end_date);
}


int calendar::dateAndTime::daysApart(std::string start_date, std::string end_date)
{
	return (boost::gregorian::from_string(end_date) - boost::gregorian::from_string(start_date)).days();
}

/*This method defines a valid date to be from 10 years ago to the current day (API Limitations).
If true the methods formats the user input to be suitable for the GET request as 'YEAR/MONTH/DAY'.
Expected user input is of the form : 'MONTH/DAY/YEAR'.*/
bool calendar::dateAndTime::isValidDate(std::string & input)
{
	if (input.size() < 6 || input.at(input.size()-1)=='/') { return false; }
	int day_num=0, month_num=0, year_num=0;
	std::string day = "", month = "", year = "";
	std::string temp = input;
	auto itr = temp.find('/');
	try {
		if (itr != std::string::npos) {
			month = temp.substr(0, itr);
			temp = temp.substr(itr + 1, temp.size() - 1);
		}
		else { return false; }
		itr = temp.find('/');
		if (itr != std::string::npos) {
			day = temp.substr(0, itr);
			temp = temp.substr(itr + 1, temp.size() - 1);
		}
		else { return false; }
		if ( (temp.size() != 2) && (temp.size() != 4)) {
			return false; } //if the year is not 2 or 4 digits it is false.
		year = temp;
		day_num = std::stoi(day,nullptr,10);
		month_num = std::stoi(month, nullptr, 10);
		year_num = std::stoi(year, nullptr, 10);
		if (year_num < 100) { year_num += current_year - (current_year % 100);} //Incase of shorthand writing of year i.e '22' add the current century. 22-->2022
		if (day_num > 31 || month_num > 12 || year_num > current_year || day_num < 1 || month_num < 1) { return false; }
		if (year_num < current_year - 10) {
			std::cout << "Error, year >" << year_num << "< exceeds the 10 year maximum limit for historical data retrieval.\n";
			return false;
		}
		if (year_num == current_year && month_num == current_month && day_num > current_day) {
			std::cout << "Error, cannot allow a Future Date.\n";
			return false;
		}
	}
	catch(std::invalid_argument const& E){ 
		std::cout << "isValidDate::Improper Date::invalid_argument.\n";
		return false; 
	}
	catch (std::out_of_range const& E) { std::cout << "isValidDate::Improper Date::out_of_range.\n"; return false; }
	if (month.size() == 1) { month = '0' + month; }
	if (day.size() == 1) { day = '0' + day; }
	input = std::to_string(year_num)+ "-" + month + "-" + day;
	return true;
}


void calendar::print(dateAndTime& dt) {
	std::cout << dt.today << "\n" << dt.yesterday << "\n";
}
