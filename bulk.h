/*
 * bulk.h
 *
 *  Created on: 30 июн. 2019 г.
 *      Author: sveta
 */
#pragma once
#include <queue>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

// сохраняет команды и  выполняет их при необходимости.
class BulkProcessor {
	std::queue<std::string> commands;
	std::string recieveTime;
	using sclock=std::chrono::system_clock;
public:
	size_t size() {
		return commands.size();
	}
	void AddCommand(std::string command) {
		if (commands.size() == 0) {
			auto currentTime = sclock::to_time_t(sclock::now());
			recieveTime = std::string(std::ctime(&currentTime));
		}
		commands.push(command);
	}

	// Выполнить команды и записать их в файл;
	void Process() {

		if (commands.size() == 0) {
			return;
		}
		std::stringstream bulkString;
		bulkString << " bulk:";
		auto front = commands.front();
		auto fileName = "bulk" + recieveTime + ".log";
		auto status = mkdir("bulkfiles", S_IRWXU | S_IRWXG);

		if (status == 0 || errno == static_cast<int>( std::errc::file_exists)) {
			fileName = "bulkfiles/" + fileName;
		}
		bulkString << front;
		commands.pop();
		while (!commands.empty()) {
			bulkString << ", " << commands.front();
			commands.pop();
		}
		auto result = bulkString.str();
		std::cout << result << std::endl;
		std::ofstream ff(fileName, std::ios_base::out);
		ff << result;
		ff.close();
		recieveTime = "";
	}

};

// получает команду и решает, что делать: выполнять или копить.
class CommandProcessor {
	const size_t N;
	BulkProcessor bulkProcessor;
	int openCount;
	int closeCount;
	const std::string openBrace = "{";
	const std::string closeBrace = "}";
public:
	size_t GetBulkSize(){
		return bulkProcessor.size();
	}
	CommandProcessor(size_t n) :
			N(n) {
		openCount = 0;
		closeCount = 0;
	}
	void ProcessCommand(std::string command) {

		// новый блок.
		if (command == openBrace) {
			if (openCount == 0) {
				bulkProcessor.Process();
			}
			openCount++;
		} else {
			// закрывающая скобка.
			if (command == closeBrace) {
				closeCount++;
				// проверка на вложенность.
				if (closeCount == openCount) {
					bulkProcessor.Process();
					openCount = 0;
					closeCount = 0;
				}
			} else {
				bulkProcessor.AddCommand(command);
				// если блок команд полностью заполнен и размер блока не был изменен скобкой.
				if (bulkProcessor.size() == N && openCount == 0) {
					bulkProcessor.Process();
				}
			}
		}
	}

};

