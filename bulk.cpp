/*
 * bulk.cpp
 *
 *  Created on: 29 июн. 2019 г.
 *      Author: sveta
 */
#include <iostream>
#include "bulk.h"
std::string trim(const std::string& str,
		const std::string& whitespace = " \t") {
	const auto strBegin = str.find_first_not_of(whitespace);
	if (strBegin == std::string::npos)
		return "";

	const auto strEnd = str.find_last_not_of(whitespace);
	const auto strRange = strEnd - strBegin + 1;

	return str.substr(strBegin, strRange);
}

int main(int, char *argv[]) {

	size_t n;

	n = 3;///std::atoi(argv[1]);
	if (n == 0)
		n = 2;

	CommandProcessor p(n);
	std::string line;
	std::getline(std::cin, line);
	while (p.processorState !=State::Finish ) {
		p.ProcessCommand(trim(line));
		std::getline(std::cin, line);
		if (std::cin.eof())
		{
			p.ProcessCommand("", true);
			break;
		}
	}

	return 0;
}
