#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <istream>
#include <stdexcept>
#include "protos.h"
#include "utility.h"

//using namespace std;

class Stringstack {
private:
	std::string argument;
	std::vector<std::string> argumentMemory;
	bool removeSpaces;
	bool finished;

public:
	friend class FourStringBin;

	// Constructors
	Stringstack();

	Stringstack(std::string input);
	Stringstack(const char *input);
	Stringstack(std::istream &input);

	Stringstack(std::string input, bool removeSpaces);
	Stringstack(const char *input, bool removeSpaces);
	Stringstack(std::istream &input, bool removeSpaces);

	// Comparison operator
	bool operator== (const std::string compare) const;

	// Static comparison methods for strings
	bool static compare(const std::string compare, const std::string compare2) {
		if ((strcasecmp(compare.c_str(), compare2.c_str())) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	bool static compare(const char *compare, const char *compare2) {
		if ((strcasecmp(compare, compare2)) == 0) {
			return true;
		}
		else {
			return false;
		}
	}

	// Removes spaces from the beginning of a string
	std::string static trim(std::string &input) {
		size_t size = input.size();
		size_t spaceCount = 0;
		bool allSpaces = true;

		if (size > 0) {
			std::string::iterator it;
			for(it = input.begin(); it != input.end(); it++, spaceCount++) {
				if (*it != ' ') {
					input = input.substr(spaceCount);
					allSpaces = false;
					break;
				}
			}
			if (allSpaces) {
				input = "";
			}
		}
		return input;
	}

	/* pop: Splits the first or next word off argument and stores it in argumentMemory.

	popDelimited: The same as pop() but is also sensitive to delimits, to handle words or commands with spaces.
	Pass it a string for both beginning and starting delimits, for example "[(" and "])".

	eraseDelimits: Decides if the delimits are erased from the final result or not.

	delimitsMatch: Decides whether the position of the delimit in the firstDelimit string is matched
	by the position of the second delimit in the secondDelimit string. For example, if "[(" and "])" are 
	passed with delimitsMatch set to true, if a [ is located, only its equivalent character in the 
	second string will be used to locate the end of the word - "]".

	popSpeech: Form of popDelimited suitable for popping strings containing " and '. Erases and matches delimits.

	find: Returns whether a specific string is contained in the argument string or not.

	findAndCount: Returns the number of instances of the specified string in argument.

	batchFind: Returns a vector of the strings that are contained within the argument string.

	batchFindAndCount: Returns a vector of std::pair containing the string and the count number of that string within
	the argument string. Does not save strings with 0 count.

	findAndPop: Locates a specific string in the string via delimits and pops it.

	findAndReplace: Locates a specific string in the string and replaces it with the replacement string.

	replaceAll: Decides whether all instances of the specific word are replaced.

	replaceReverseOrder: Decides whether the string is traversed in order or in reverse.

	findAndReplaceOnce: findAndReplace which never has replaceAll set true.
	*/
	std::string pop();
	std::string popDelimited(std::string firstDelimit, std::string secondDelimit);
	std::string popDelimited(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits, bool delimitsMatch);
	std::string popSpeech();

	bool find(std::string specifier);
	int findAndCount(std::string specifier);
	std::vector<std::string> batchFind(const std::vector<std::string> &strings);
	std::vector<std::pair<std::string, int> > batchFindAndCount(const std::vector<std::string> &strings);

	std::string findAndPop(std::string firstDelimit, std::string secondDelimit);
	std::string findAndPop(std::string firstDelimit, std::string secondDelimit, bool eraseDelimits);

	void findAndReplace(std::string specifier, std::string replacement);
	void findAndReplace(std::string specifier, std::string replacement, bool replaceAll, bool replaceReverseOrder);
	void findAndReplaceOnce(std::string specifier, std::string replacement);
	void findAndReplaceOnce(std::string specifier, std::string replacement, bool replaceReverseOrder);

	/* Recall past arguments
	Origin at zero */
	std::string recall(size_t indexArg);
	std::string last();

	// Get integers/doubles from the string
	int toInt();
	int toInt(size_t indexArg);
	double toDouble();
	double toDouble(size_t indexArg);

	/* Return portions of the stored argument, either from an argument, between arguments or all of them
	Origin at zero */
	std::string printPartialArg(size_t begin);
	std::string printPartialArg(size_t begin, size_t arguments);
	std::string printArg();

	// Pops the whole input, either with pop or popSpeech
	void popAll();
	void popAllSpeech();

	// Returns if the Stringstack is fully popped
	bool isFinished();

	// Returns the current argument string
	std::string getRemainingArgument();
};

class FourStringBin {
private:
	std::string premote;
	std::string command;
	std::string emote;
	std::string remains;

public:
	Stringstack rawArgument;

	// Constructors
	FourStringBin();
	FourStringBin(std::string input);
	FourStringBin(std::istream &input);

	void processCommand();
	void processEmotes();
	std::string processRemains();
	void processFullBin();

	std::string pop();

	std::string getPremote();
	std::string getPremoteProper();
	std::string getCommand();
	std::string getEmote();
	std::string getRemains();
	std::string getRemainsProper();
	Stringstack getArgument();
};