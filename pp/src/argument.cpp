#include "argument.h"

/*********************************************************
*          Stringstack Function Definitions              *
*                (formerly Argument)                     *
*                   Case                                 *
*********************************************************/
Stringstack::Stringstack() {
}

Stringstack::Stringstack(std::string input) {
	argument = input;
	removeSpaces = true;
	finished = false;
	
	Stringstack::trim(argument);
}

Stringstack::Stringstack(const char *input) {
	std::string temp = input;
	
	argument = temp;
	removeSpaces = true;
	finished = false;

	Stringstack::trim(argument);
}

Stringstack::Stringstack(std::istream &input) {
	std::string temp;
	
	getline(input, temp);
	argument = temp;
	removeSpaces = true;
	finished = false;

	Stringstack::trim(argument);
}


Stringstack::Stringstack(std::string input, bool removeSpaces) {
	argument = input;
	this->removeSpaces = removeSpaces;
	finished = false;
	
	if (removeSpaces) {
		Stringstack::trim(argument);
	}
}

Stringstack::Stringstack(const char *input, bool removeSpaces) {
	argument = input;
	this->removeSpaces = removeSpaces;
	finished = false;

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
}

Stringstack::Stringstack(std::istream &input, bool removeSpaces) {
	std::string temp;
	
	getline(input, temp);
	argument = temp;
	this->removeSpaces = removeSpaces;
	finished = false;

	if (removeSpaces) {
		Stringstack::trim(argument);
	}
}

bool Stringstack::operator== (const std::string compare) const {
	if (!(this->argumentMemory.empty())) {
		if ((strcasecmp(this->argumentMemory.back().c_str(), compare.c_str())) == 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

std::string Stringstack::pop() {
	if (removeSpaces) {
		Stringstack::trim(argument);
	}

	if (finished) {
		return "";
	}

	size_t space = argument.find_first_of(' ');

	if (space != std::string::npos) {
		if (argument.size() > space + 1) {
			argumentMemory.push_back(argument.substr(0,space));
			argument = argument.substr(space + 1);
			return argumentMemory.back();
		}
		else {
			argumentMemory.push_back(argument.substr(0,space));
			argument = "";
			finished = true;
			return argumentMemory.back();
		}
	}
	else {
		if (argument != "" || argumentMemory.empty()) {
			argumentMemory.push_back(argument);
		}
		argument = "";
		finished = true;
		return argumentMemory.back();
	}
}

std::string Stringstack::popDelimited(std::string firstDelimit, std::string secondDelimit) {
	return popDelimited(firstDelimit, secondDelimit, true, false);
}

std::string Stringstack::popDelimited(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits, bool delimitsMatch) {
	pop();
	std::string resultString = argumentMemory.back();
	size_t size = resultString.size();
	size_t tempPos = resultString.find_first_of(firstDelimit);

	if (tempPos != std::string::npos) {
		if (delimitsMatch) {
			size_t delimitPos = secondDelimit.find_first_of(resultString[tempPos]);
			if (delimitPos == std::string::npos) {
				delimitPos = firstDelimit.find_first_of(resultString[tempPos]);
				if (delimitPos < secondDelimit.size()) {
					secondDelimit = secondDelimit[delimitPos];
				}
				else {
					throw std::runtime_error("Incorrectly matched delimits on call to popDelimited");
					return argumentMemory.back();
				}
			}
			else {
				secondDelimit = resultString[tempPos];
			}
		}
		if (eraseDelimits) {
			resultString.erase(tempPos,1);
		}
		tempPos = resultString.find_last_of(secondDelimit);
		if (tempPos != std::string::npos) {
			if (eraseDelimits) {
				resultString.erase(tempPos,1);
			}
		}
		else {
			tempPos = argument.find_first_of(secondDelimit);
			if (!eraseDelimits) {
				tempPos++;
			}
			if (tempPos != std::string::npos) {
				resultString.append(" " + argument.substr(0, tempPos));
				size = argument.size();

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
	argumentMemory.back() = resultString;
	return resultString;
}

std::string Stringstack::popSpeech() {
	return popDelimited("\"'", "\"'", true, true);
}

bool Stringstack::find(std::string specifier) {
	return (argument.find(specifier) != std::string::npos);
}

int Stringstack::findAndCount(std::string specifier) {
	int output = 0;
	size_t position = argument.find(specifier);

	while (position != std::string::npos) {
		output++;
		position = argument.find(specifier, position);
	}
	return output;
}

std::vector<std::string> Stringstack::batchFind(const std::vector<std::string> &strings) {
	std::vector<std::string> output;

	if (strings.size() > 0) {
		size_t stringsSize = strings.size();

		for (size_t i = 0; i < stringsSize; i++) {
			if (find(strings[i])) {
				output.push_back(strings[i]);
			}
		}
	}
	return output;
}

std::vector<std::pair<std::string, int> > Stringstack::batchFindAndCount(const std::vector<std::string> &strings) {
	std::vector<std::pair<std::string, int> > output;
	
	if (strings.size() > 0) {
		size_t stringsSize = strings.size();
		int tempCount = 0;

		for (size_t i = 0; i < stringsSize; i++) {
			tempCount = findAndCount(strings[i]);
			if (tempCount > 0) {
				output.push_back(std::pair<std::string, int>(strings[i], tempCount));
			}
		}
	}
	return output;
}

std::string Stringstack::findAndPop(std::string firstDelimit, std::string secondDelimit) {
	return findAndPop(firstDelimit, secondDelimit, true);
}

std::string Stringstack::findAndPop(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits) {
	size_t firstPos = argument.find(firstDelimit);
	size_t secondPos = argument.find(secondDelimit, firstPos);

	if ((firstPos != std::string::npos) && (secondPos != std::string::npos)) { //&& (secondPos > firstPos)) {
		argumentMemory.push_back(argument.substr((firstPos + 1), (secondPos - firstPos - 1)));
		if (eraseDelimits) {
			argument.erase(firstPos, argumentMemory.back().size() + 2);
		}
		else {
			argument.erase(firstPos + 1, argumentMemory.back().size() + 1);
		}
		return argumentMemory.back();
	}
	return "";
}

void Stringstack::findAndReplace(std::string specifier, std::string replacement) {
	findAndReplace(specifier, replacement, true, false);
}

void Stringstack::findAndReplace(std::string specifier, std::string replacement, bool replaceAll, bool replaceReverseOrder) {
	size_t specifierPos = 0;
	size_t specifierSize = specifier.size();
	bool finished = false;

	if (replaceReverseOrder) {
		specifierPos = (argument.size() - 1);
	}

	do {
		if (replaceReverseOrder) {
			specifierPos = argument.rfind(specifier, specifierPos);
		}
		else {
			specifierPos = argument.find(specifier, specifierPos);
		}

		if (specifierPos == std::string::npos) {
			break;
		}

		argument.replace(specifierPos, specifierSize, replacement);

		if (!replaceReverseOrder) {
			specifierPos += replacement.size();
		}
	}while (replaceAll);
}

void Stringstack::findAndReplaceOnce(std::string specifier, std::string replacement) {
	findAndReplace(specifier, replacement, false, false);
}

void Stringstack::findAndReplaceOnce(std::string specifier, std::string replacement, bool replaceReverseOrder) {
	findAndReplace(specifier, replacement, false, replaceReverseOrder);
}

std::string Stringstack::recall(size_t indexArg) {
	if (indexArg < argumentMemory.size()) {
		return argumentMemory[indexArg];
	}
	else {
		return "";
	}
}

std::string Stringstack::last() {
	if (argumentMemory.size() > 0) {
		return argumentMemory.back();
	}
	else {
		return "";
	}
}

int Stringstack::toInt() {
	if (argumentMemory.size() > 0 && argumentMemory.back().size() > 0) {
		return atoi(argumentMemory.back().c_str());
	}
	else {
		return -1;
	}
}

int Stringstack::toInt(size_t indexArg) {
	return atoi((recall(indexArg)).c_str());
}

double Stringstack::toDouble() {
	if (argumentMemory.size() > 0 && argumentMemory.back().size() > 0) {
		return atof(argumentMemory.back().c_str());
	}
	else {
		return -1.0;
	}
}

double Stringstack::toDouble(size_t indexArg) {
	return atof((recall(indexArg)).c_str());
}

std::string Stringstack::printPartialArg(size_t begin) {
	size_t size = argumentMemory.size();
	std::stringstream toReturn;

	if (begin >= size) {
		return "ERROR: No Argument There At That Position";
	}
	else {
		for (size_t i = begin; i <= size; i++) {
			toReturn << recall(i);
			if (i < (size - 1)) {
				toReturn << " ";
			}
		}
	}
	return toReturn.str();
}

std::string Stringstack::printPartialArg(size_t begin, size_t arguments) {
	size_t size = argumentMemory.size();
	std::stringstream toReturn;

	if (begin >= size) {
		return printPartialArg(0);
	}
	else {
		for (size_t i = begin; i < size && arguments; i++) {
			toReturn << recall(i);
			if (i < (size - 1) && i < arguments) {
				toReturn << " ";
			}
			arguments--;
		}
	}
	return toReturn.str();
}

std::string Stringstack::printArg() {
	return printPartialArg(0);
}

void Stringstack::popAll() {
	while (!finished) {
		pop();
	}
}

void Stringstack::popAllSpeech() {
	while (!finished) {
		popSpeech();
	}
}

bool Stringstack::isFinished() {
	return finished;
}

std::string Stringstack::getRemainingArgument() {
	return argument;
}

/*********************************************************
*          FourStringBin Function Definitions            *
*                                                        *
*                   Case                                 *
*********************************************************/
FourStringBin::FourStringBin() {
}

FourStringBin::FourStringBin(std::string input) {
	rawArgument = input;
	Stringstack::trim(rawArgument.argument);
}

FourStringBin::FourStringBin(std::istream &input) {
	std::string temp;

	getline(input, temp);
	rawArgument = temp;
	Stringstack::trim(rawArgument.argument);
}

void FourStringBin::processCommand() {
	if (getCommand().empty()) {
		command = rawArgument.popDelimited("[(","])", true, true);
	}
	else {
		premote = command;
		command = rawArgument.pop();
	}
}

void FourStringBin::processEmotes() {
	if (premote.empty()) {
		premote = rawArgument.findAndPop("[", "]");
	}
	emote = rawArgument.findAndPop("(", ")");
}

std::string FourStringBin::processRemains() {
	remains = Stringstack::trim(rawArgument.argument);
	return remains;
}

void FourStringBin::processFullBin() {
	processCommand();
	processEmotes();
}

std::string FourStringBin::pop() {
	std::string toReturn = rawArgument.pop();
	processRemains();
	return toReturn;
}

std::string FourStringBin::getPremote() {
	if (premote.empty()) {
		return "";
	}
	else {
		return premote;
	}
}

std::string FourStringBin::getPremoteProper() {
	if (premote.empty()) {
		return "";
	}
	else {
		char tempFirstChar = (char)toupper(premote[0]);
		return (tempFirstChar + premote.substr(1));
	}
}

std::string FourStringBin::getCommand() {
	if (command.empty()) {
		return "";
	}
	else {
		return command;
	}
}

std::string FourStringBin::getEmote() {
	if (emote.empty()) {
		return "";
	}
	else {
		return emote;
	}
}

std::string FourStringBin::getRemains() {
	if (remains.empty()) {
		return "";
	}
	return remains;
}

std::string FourStringBin::getRemainsProper() {
	if (remains.empty()) {
		return "";
	}
	char tempFirstChar = (char)toupper(remains[0]);

	return (tempFirstChar + remains.substr(1));
}

Stringstack FourStringBin::getArgument() {
	return rawArgument;
}
