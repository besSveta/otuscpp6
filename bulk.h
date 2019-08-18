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

class FilerWriter {
public:
	void Write(std::string recieveTime, const std::stringstream &bulkString) {
		auto fileName = "bulk" + recieveTime + ".log";
		std::ofstream ff(fileName, std::ios_base::out);
		ff << bulkString.str();
		ff.close();
	}
};

class ConsoleWriter {
public:
	void Write(const std::stringstream &bulkString) {
		std::cout << bulkString.str() << std::endl;
	}
};
enum class State {
	Processed, Finish, WaitComand,
};

// получает команду и решает, что делать: выполнять или копить.
class CommandProcessor {
	const size_t N;
	int openCount;
	int closeCount;
	sclock::time_point prevTime;
	const std::string openBrace = "{";
	const std::string closeBrace = "}";

	std::queue<std::string> commands;
	std::string recieveTime;

	ConsoleWriter consoleW;
	FilerWriter fileW;

	void AddCommand(std::string command, std::string time) {
		if (commands.size() == 0) {
			recieveTime = time;
		}
		commands.push(command);
	}
	void Process() {
		std::stringstream bulkString;
		bulkString << " bulk:";
		bulkString << commands.front();
		commands.pop();
		while (!commands.empty()) {
			bulkString << ", " << commands.front();
			commands.pop();
		}
		consoleW.Write(bulkString);
		fileW.Write( recieveTime,bulkString);
	}
public:
	State processorState;

	CommandProcessor(size_t n) :
			N(n) {
		openCount = 0;
		closeCount = 0;
		prevTime = sclock::now();
		processorState = State::WaitComand;
	}

	size_t GetBulkSize() {
		return commands.size();
	}
	void ProcessCommand(std::string command, bool isLast=false) {

		auto currentTime = sclock::now();
		auto recieveTime = std::to_string(
				std::chrono::duration_cast<std::chrono::microseconds>(
						currentTime.time_since_epoch()).count());
		auto diff = std::chrono::duration_cast<std::chrono::seconds>(
				currentTime - prevTime).count();
		// выйти при вводе пустой строки и интервале между командами >2 секунд.
		if (command == "" && diff > 2) {
			processorState = State::Finish;
			return;
		}
		prevTime = currentTime;
		// новый блок.
		if (command == openBrace) {
			if (openCount == 0) {
				Process();

				processorState = State::Processed;
			}
			openCount++;
		} else {
			// закрывающая скобка.
			if (command == closeBrace && openCount > 0) {
				closeCount++;
				// проверка на вложенность.
				if (closeCount == openCount) {
					Process();
					openCount = 0;
					closeCount = 0;
					processorState = State::Processed;
				}
			} else {
				if (isLast){
					Process();
				   processorState = State::Finish;
				   return;
				}
				AddCommand(command, recieveTime);
				// если блок команд полностью заполнен и размер блока не был изменен скобкой.
				if (commands.size() == N && openCount == 0) {
					Process();
					processorState = State::Processed;
				} else {
					processorState = State::WaitComand;
				}
			}
		}
	}

};

