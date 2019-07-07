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

using sclock=std::chrono::system_clock;
// сохраняет команды и  выполняет их при необходимости.
class BulkProcessor {
	std::queue<std::string> commands;
	std::string recieveTime;
public:
	size_t size() {
		return commands.size();
	}
	void AddCommand(std::string command, std::string time) {
		if (commands.size() == 0) {
			recieveTime = time;
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
 enum class State{
	Processed,
	Finish,
	WaitComand,
};

// получает команду и решает, что делать: выполнять или копить.
class CommandProcessor {
	const size_t N;
	BulkProcessor bulkProcessor;
	int openCount;
	int closeCount;
	sclock::time_point prevTime;
	const std::string openBrace = "{";
	const std::string closeBrace = "}";
public:
	State processorState;
	size_t GetBulkSize(){
		return bulkProcessor.size();
	}
	CommandProcessor(size_t n) :
			N(n) {
		openCount = 0;
		closeCount = 0;
		prevTime=sclock::now();
		processorState= State::WaitComand;
	}

	void ProcessCommand(std::string command) {

		auto currentTime=sclock::now();
		auto recieveTime = std::to_string(std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()).count());
		auto diff=std::chrono::duration_cast<std::chrono::seconds>(currentTime - prevTime).count();
		// выйти при вводе пустой строки и интервале между командами >2 секунд.
		if (command=="" && diff>2){
			processorState=State::Finish;
			return;
		}
		prevTime=currentTime;
		// новый блок.
		if (command == openBrace) {
			if (openCount == 0) {
				bulkProcessor.Process();
				processorState=State::Processed;
			}
			openCount++;
		} else {
			// закрывающая скобка.
			if (command == closeBrace && openCount>0) {
				closeCount++;
				// проверка на вложенность.
				if (closeCount == openCount) {
					bulkProcessor.Process();
					openCount = 0;
					closeCount = 0;
					processorState=State::Processed;
				}
			} else {
				bulkProcessor.AddCommand(command, recieveTime);
				// если блок команд полностью заполнен и размер блока не был изменен скобкой.
				if (bulkProcessor.size() == N && openCount == 0) {
					bulkProcessor.Process();
					processorState=State::Processed;
				}
				else {
					processorState=State::WaitComand;
				}
			}
		}
	}

};

