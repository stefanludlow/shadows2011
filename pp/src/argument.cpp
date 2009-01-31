#include <vector>
#include <sstream>
#include <iostream>

const size_t ARGUMENT_SQUELCH_LIMIT = 75; // For one Arg

class Argument {
private:
	std::string argument;
	std::vector<std::string> argumentMemory;
	bool removeSpaces;

	void squelch() {
		size_t size = argument.size();
		size_t squelchLimit = ARGUMENT_SQUELCH_LIMIT;

		if (size > 0) {
			std::string::iterator it;
			for(it = argument.begin(); it != argument.end() && squelchLimit; squelchLimit--) {
				if (*it != ' ') {
					break;
				}
				else {
					argument.erase(it);
				}
			}
			size = argument.size();
			if (size <= 0) {
				argument = "";
			}
		}
	}

public:
	Argument(std::string input) {
		argument = input;
		removeSpaces = true;
		
		squelch();
	}

	Argument(std::string input, bool removeSpaces) {
		argument = input;
		this->removeSpaces = removeSpaces;
		
		if (removeSpaces) {
			squelch();
		}
	}


	bool Argument::operator== (const std::string compare) const {
		if (!(this->argumentMemory.empty())) {
			if ((_stricmp(this->argumentMemory.back().c_str(), compare.c_str())) == 0) {
				return true;
			}
		}
		else {
			return false;
		}
	}

	bool static compare(const std::string compare, const std::string compare2) {
		if ((_stricmp(compare.c_str(), compare2.c_str())) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	std::string pop() {
		if (removeSpaces) {
			squelch();
		}

		size_t space = argument.find_first_of(' ');

		if (space != std::string::npos) {
			if (argument.length() > space + 1) {
				argumentMemory.push_back(argument.substr(0,space));
				argument = argument.substr(space + 1);
				return argumentMemory.back();
			}
			else {
				argumentMemory.push_back(argument.substr(0,space));
				argument = "";
				return argumentMemory.back();
			}
		}
		else {
			if (argument != "" || argumentMemory.empty()) {
				argumentMemory.push_back(argument);
			}
			argument = "";
			return argumentMemory.back();
		}
	}

	std::string popSpeech() {
		pop();
		std::string toReturn = argumentMemory.back();
		size_t size = toReturn.length();
		size_t tempPos = toReturn.find_first_of('"');

		if (tempPos != std::string::npos) {
			toReturn.erase(tempPos,1);
			tempPos = toReturn.find_last_of('"');
			if (tempPos != std::string::npos) {
				toReturn.erase(tempPos,1);
			}
			else {
				tempPos = argument.find_first_of('"');
				if (tempPos != std::string::npos) {
					std::stringstream concatenate;
					
					argument.erase(tempPos,1);
					concatenate << toReturn << " " << argument.substr(0, tempPos);
					toReturn = concatenate.str();
					size = argument.length();
					if (tempPos >= size) {
						argument = "";
					}
					else {
						argument = argument.substr(tempPos + 1);
					}
				}
				else {
					return argumentMemory.back();
				}
			}
		}
		else {
			size_t tempPos = toReturn.find_first_of("'");

			if (tempPos != std::string::npos) {
				toReturn.erase(tempPos,1);
				tempPos = toReturn.find_last_of("'");
				if (tempPos != std::string::npos) {
					toReturn.erase(tempPos,1);
				}
				else {
					tempPos = argument.find_first_of("'");
					if (tempPos != std::string::npos) {
						std::stringstream concatenate;

						argument.erase(tempPos,1);
						concatenate << toReturn << " " << argument.substr(0, tempPos);
						toReturn = concatenate.str();
						size = argument.length();
						if (tempPos >= size) {
							argument = "";
						}
						else {
							argument = argument.substr(tempPos + 1);
						}
					}
					else {
						return argumentMemory.back();
					}
				}
			}
		}
		argumentMemory.back() = toReturn;
		return toReturn;
	}

	std::string printPartialArg(size_t begin) {
		size_t size = argumentMemory.size();
		std::stringstream toReturn;

		if (begin >= size) {
			return "ERROR: No Argument There At That Position";
		}
		else {
			for (size_t i = begin; i <= size; i++) {
				toReturn << recall(i);
				if (i < size) {
					toReturn << " ";
				}
			}
		}
		return toReturn.str();
	}

	std::string printPartialArg(size_t begin, size_t arguments) {
		size_t size = argumentMemory.size();
		std::stringstream toReturn;

		if (begin >= size) {
			return (printPartialArg(0));
		}
		else {
			for (size_t i = begin; i < size && arguments; i++) {
				toReturn << recall(i);
				if (i <= size  && i <= arguments) {
					toReturn << " ";
				}
				arguments--;
			}
		}
		return toReturn.str();
	}

	std::string recall(size_t indexArg) {
		if (indexArg < argumentMemory.size()) {
			return argumentMemory[indexArg];
		}
		else {
			return "";
		}
	}

	void popAll() {
		while (argument != "") {
			pop();
		}
	}

	void popAllSpeech() {
		while (argument != "") {
			popSpeech();
		}
	}
};

int main() {
	Argument input ("             clan set black_watch member", false);
	
	if (Argument::compare("wassup", "wassup")) {
		std::cout << "Yay!" << std::endl;
	}
	input.popAllSpeech();
	std::cout << input.printPartialArg(0) << std::endl;
}
