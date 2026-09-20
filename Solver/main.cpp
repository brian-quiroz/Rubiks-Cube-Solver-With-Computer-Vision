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

void autoScramble(std::string filename);
void manualScramble();
bool isValidAlgorithm(std::string userAlgorithm);
bool isValidMove(std::string move);

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "Please run with \"auto [stickerFile]\" to read cube from file or \"manual\" to input scramble algorithm manually!\n";
		return 1;
	}

	try {
		if (strcmp(argv[1],"auto") == 0) {
			autoScramble(argc > 2 ? argv[2] : "../Detector/stickers.txt");
		} else if (strcmp(argv[1],"manual") == 0) {
			manualScramble();
		} else {
			std::cout << "Wrong argument!\n";
			return 1;
		}
	} catch (std::runtime_error& e) {
		std::cout << "\nError: " << e.what() << '\n';
		return 1;
	}

	return 0;
}

void autoScramble(std::string filename) {
	Solver s("auto", filename);
}

void manualScramble() {
	std::string userAlgorithm = "";

	std::cout << "\nUser userAlgorithm: ";

	std::getline(std::cin, userAlgorithm);

	while (!isValidAlgorithm(userAlgorithm)) {
		if (std::cin.eof()) {
			throw std::runtime_error("No valid userAlgorithm was given.");
		}
		std::cout << "Error! Please input a valid userAlgorithm: ";
		std::getline(std::cin, userAlgorithm);
	}

	Solver s("manual", userAlgorithm);
}

//A valid userAlgorithm is one or more moves separated by single spaces.
bool isValidAlgorithm(std::string userAlgorithm) {
	std::string move = "";

	for (unsigned int i = 0; i <= userAlgorithm.length(); i++) {
		if ((i == userAlgorithm.length()) || (userAlgorithm[i] == ' ')) {
			if (!isValidMove(move)) {
				return false;
			}
			move = "";
		} else {
			move += userAlgorithm[i];
		}
	}

	return true;
}

//A valid move is F, R, U, D, L or B, optionally followed by ' or 2.
bool isValidMove(std::string move) {
	if ((move.length() == 0) || (move.length() > 2)) {
		return false;
	}
	if ((move[0] != 'F') && (move[0] != 'R') && (move[0] != 'U') && (move[0] != 'D') && (move[0] != 'L') && (move[0] != 'B')) {
		return false;
	}
	return ((move.length() == 1) || (move[1] == '\'') || (move[1] == '2'));
}
