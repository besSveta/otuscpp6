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

	n = std::atoi(argv[0]);
	if (n == 0)
		n = 2;

	CommandProcessor p(n);
	for (std::string line; (p.processorState !=State::Finish && std::getline(std::cin, line));) {
		p.ProcessCommand(trim(line));
	}
	return 0;
}
