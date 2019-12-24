#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <unordered_map>

#include "Piece.h"
#include "Solver.h"
#include "Node.h"
#include "Queue.h"
#include "LinkedList.h"
#include "PreconditionViolationException.h"

void autoScramble();
void manualScramble();

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Please run with \"auto\" to read cube from file or \"manual\" to input scramble algorithm manually!\n";
		return 0;
	}

	if (strcmp(argv[1],"auto") == 0) {
		autoScramble();
	} else if (strcmp(argv[1],"manual") == 0) {
		manualScramble();
	} else {
		std::cout << "Wrong argument!\n";
	}

	return 0;
}

void autoScramble() {
	std::string filename = "../Detector/stickers.txt";
	Solver s("auto", filename);
}

void manualScramble() {
	bool isValid = true;
	std::string userAlgorithm = "";

	std::cout << "\nUser userAlgorithm: ";

	std::getline(std::cin, userAlgorithm);

	if (userAlgorithm.length() > 1) {
		isValid = false;
		for (unsigned int i = 0; i < userAlgorithm.length(); i++) {
			if (userAlgorithm[i] == ' ') {
				isValid = true;
			}
		}
	}

	while (((userAlgorithm[0] != 'F') && (userAlgorithm[0] != 'R') && (userAlgorithm[0] != 'U') && (userAlgorithm[0] != 'D') &&
			(userAlgorithm[0] != 'B') && (userAlgorithm[0] != 'L')) || (!isValid)) {
		std::cout << "Error! Please input a valid userAlgorithm: ";
		std::getline(std::cin, userAlgorithm);
		isValid = true;
		if (userAlgorithm.length() > 1) {
			isValid = false;
			for (unsigned int i = 0; i < userAlgorithm.length(); i++) {
				if (userAlgorithm[i] == ' ') {
					isValid = true;
				}
			}
		}
	}

	Solver s("manual", userAlgorithm);
}
