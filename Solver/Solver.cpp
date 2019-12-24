#include "Solver.h"

Solver::Solver(std::string mode, std::string scramble) {
	int pos = 0;

	//This 2D array will simulate a "spread out" cube in two dimensions.
	cube = new Piece**[9];
	for (int i = 0; i < 9; i++) {
		cube[i] = new Piece*[12];
	}

	//Creating a new "piece" object at every position in the grid.
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 12; j++) {
			cube[i][j] = new Piece();
		}
	}

	//Assigning a "permutation number" to every piece that corresponds to a piece on the cube (not all the pieces in the array will correspond to a piece in the cube) so that I will be able to keep track of each individual piece.
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 12; j++) {
			if ((i < 3 && j > 2 && j < 6) || (i >= 3 && i <= 5) || (i > 5 && j > 2 && j < 6)) {
				pos++;
				cube[i][j]->setPermutation(pos);
			}
		}
	}

	std::cout << "\nCube in solved state (initial):\n";
	printCube();

	//Auto Scramble
	if (mode == "auto") {
		autoScramble(scramble);
	//Manual Scramble
	} else if (mode == "manual") {
		manualScramble(scramble);
	}

	int queueSize = solverAlg.getLength();
	for (int i = 0; i < queueSize; i++) {
		solverAlg.dequeue();
	}
	//std::cout << "\nSOLVER INITIAL: " << solverAlg.getLength() << std::endl;

	std::cout << "\nCube in scrambled state:\n ";
	printCube();

	std::cout << "\n\nSOLVING CUBE...\n";
	Cross();
	FirstLayerF2L();
	SecondLayerF2L();
	EdgesOLL();
	EdgesPLL();
	CornersPLL();
	CornersOLL();

	std::cout << "\n\nSOLVED!\n";

	//std::cout << "\nNumber of moves used by program: " << solverAlg.getLength() << std::endl;

	simplifyAlgorithm(solverAlg);

	std::cout << "\nNumber of moves after being simplified: " << solverAlg.getLength() << std::endl;

	queueSize = solverAlg.getLength();

	std::cout << "\nAlgorithm translated into Rubik's cube notation:\n";
	if (queueSize != 0) {
		for (int i = 0; i < queueSize - 1; i++) {
			std::cout << solverAlg.peekFront() << " ";
			solverAlg.dequeue();
		}
		std::cout << solverAlg.peekFront();
		solverAlg.dequeue();
		std::cout << std::endl;
	} else {
		std::cout << "No moves!\n";
	}

	std::cout << std::endl;
}

Solver::~Solver() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 12; j++) {
			delete cube[i][j];
		}
	}
	for (int i = 0; i < 9; i++) {
		delete[] cube[i];
	}
	delete[] cube;
}

void Solver::autoScramble(std::string filename) {
	std::ifstream inFile;

	inFile.open(filename);

	colors = new char*[6];
	for (int i = 0; i < 6; i++) {
		colors[i] = new char[9];
	}

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 9; j++) {
			inFile >> colors[i][j];
		}
	}

	inFile.close();

	identifyPieces(colors);

	for (int i = 0; i < 6; i++) {
		delete[] colors[i];
	}
	delete[] colors;
}

void Solver::manualScramble(std::string userAlgorithm) {
	int size = 0;
	std::string temp =  "";

	//Iterate through the user's input, adding strings that were originally separated by spaces individually to my scramblerAlg queue, ignoring all of the spaces.
	for (unsigned int i = 0; i < userAlgorithm.length(); i++) {
		if (userAlgorithm[i] != ' ') {
			temp += userAlgorithm[i];
		} else {
			scramblerAlg.enqueue(temp);
			temp = "";
		}
	}
	scramblerAlg.enqueue(temp);

	simplifyAlgorithm(scramblerAlg);

	size = scramblerAlg.getLength();

	std::cout << "\nApplying scrambler algorithm: " << '\n';

	for (int i = 0; i < size; i++) {
		std::cout << scramblerAlg.peekFront() << " ";
		if (scramblerAlg.peekFront() == "F") {
			F();
		} else if (scramblerAlg.peekFront() == "R") {
			R();
		} else if (scramblerAlg.peekFront() == "U") {
			U();
		} else if (scramblerAlg.peekFront() == "D") {
			D();
		} else if (scramblerAlg.peekFront() == "L") {
			L();
		} else if (scramblerAlg.peekFront() == "B") {
			B();
		} else if (scramblerAlg.peekFront() == "E") {
			E();
		} else if (scramblerAlg.peekFront() == "F'") {
			Fp();
		} else if (scramblerAlg.peekFront() == "R'") {
			Rp();
		} else if (scramblerAlg.peekFront() == "U'") {
			Up();
		} else if (scramblerAlg.peekFront() == "D'") {
			Dp();
		} else if (scramblerAlg.peekFront() == "L'") {
			Lp();
		} else if (scramblerAlg.peekFront() == "B'") {
			Bp();
		} else if (scramblerAlg.peekFront() == "E'") {
			Ep();
		} else if (scramblerAlg.peekFront() == "R2") {
			R2();
		} else if (scramblerAlg.peekFront() == "U2") {
			U2();
		} else if (scramblerAlg.peekFront() == "D2") {
			D2();
		} else if (scramblerAlg.peekFront() == "L2") {
			L2();
		} else if (scramblerAlg.peekFront() == "B2") {
			B2();
		} else if (scramblerAlg.peekFront() == "E2") {
			E2();
		}
		scramblerAlg.dequeue();
	}
	std::cout << std::endl;
}

void Solver::identifyPieces(char** c) {
	std::vector<std::vector <std::pair<int, int>>> stdCornersCoords = {{{0,3}, {3,0}, {3,11}},
									   {{0,5}, {3,8}, {3,9}},
									   {{2,3}, {3,2}, {3,3}},
									   {{2,5}, {3,5}, {3,6}},
									   {{6,3}, {5,2}, {5,3}},
									   {{6,5}, {5,5}, {5,6}},
									   {{8,3}, {5,0}, {5,11}},
									   {{8,5}, {5,8}, {5,9}}};

	std::vector<std::vector <std::pair<int, int>>> stdEdgesCoords = {{{0,4}, {3,10}}, {{1,3}, {3,1}},
									 {{1,5}, {3,7}}, {{2,4}, {3,4}},
									 {{6,4}, {5,4}}, {{7,3}, {5,1}},
									 {{7,5}, {5,7}}, {{8,4}, {5,10}},
									 {{4,0}, {4,11}}, {{4,3}, {4,2}},
									 {{4,6}, {4,5}}, {{4,9}, {4,8}}};

	std::vector<std::vector<char>> cornersGiven = {{c[0][0], c[1][0], c[4][2]},
						       {c[0][2], c[3][2], c[4][0]},
						       {c[0][6], c[1][2], c[2][0]},
						       {c[5][0], c[1][8], c[2][6]},
						       {c[5][2], c[2][8], c[3][6]},
						       {c[5][6], c[1][6], c[4][8]},
						       {c[5][8], c[3][8], c[4][6]}};

	std::vector<std::vector<char>> edgesGiven = {{c[0][1], c[4][1]}, {c[0][3], c[1][1]},
						     {c[0][5], c[3][1]}, {c[0][7], c[2][1]},
						     {c[5][1], c[2][7]}, {c[5][3], c[1][7]},
						     {c[5][5], c[3][7]}, {c[5][7], c[4][7]},
						     {c[1][3], c[4][5]}, {c[2][3], c[1][5]},
						     {c[3][3], c[2][5]}, {c[4][3], c[3][5]}};

	std::vector<int> cornersIndexes(8,0);
	std::vector<int> edgesIndexes(12,0);
	std::vector<int> cornersPerm(8,0);
	std::vector<int> edgesPerm(12,0);

	for (unsigned int i = 0; i < cornersGiven.size(); i++) {
		std::tie(cornersIndexes[i], cornersPerm[i]) = identifyCorner(cornersGiven[i]);
	}

	for (unsigned int i = 0; i < edgesGiven.size(); i++) {
		std::tie(edgesIndexes[i], edgesPerm[i]) = identifyEdge(edgesGiven[i]);
	}

	std::unordered_map<int, int> old2NewStickers;
	int oldCorner = 0, newCorner = 0, oldEdge = 0, newEdge = 0;

	for (unsigned int i = 0; i < cornersGiven.size(); i++) {
		std::vector<std::pair<int,int>> permutedCornerCoords = cornerStickerPermN(stdCornersCoords[cornersIndexes[i]], cornersPerm[i]);

		for (unsigned int j = 0; j < stdCornersCoords[i].size(); j++) {
			oldCorner = cube[stdCornersCoords[i][j].first][stdCornersCoords[i][j].second]->getPermutation();
			newCorner = cube[permutedCornerCoords[j].first][permutedCornerCoords[j].second]->getPermutation();
			old2NewStickers.insert(std::make_pair(oldCorner,newCorner));
		}
	}

	for (unsigned int i = 0; i < edgesGiven.size(); i++) {
		std::vector<std::pair<int,int>> permutedEdgeCoords = edgeStickerPermN(stdEdgesCoords[edgesIndexes[i]], edgesPerm[i]);

		for (unsigned int j = 0; j < stdEdgesCoords[i].size(); j++) {
			oldEdge = cube[stdEdgesCoords[i][j].first][stdEdgesCoords[i][j].second]->getPermutation();
			newEdge = cube[permutedEdgeCoords[j].first][permutedEdgeCoords[j].second]->getPermutation();
			old2NewStickers.insert(std::make_pair(oldEdge,newEdge));
		}
	}

	//Insert centers:
	old2NewStickers.insert(std::make_pair(cube[1][4]->getPermutation(),cube[1][4]->getPermutation()));
	old2NewStickers.insert(std::make_pair(cube[4][1]->getPermutation(),cube[4][1]->getPermutation()));
	old2NewStickers.insert(std::make_pair(cube[4][4]->getPermutation(),cube[4][4]->getPermutation()));
	old2NewStickers.insert(std::make_pair(cube[4][7]->getPermutation(),cube[4][7]->getPermutation()));
	old2NewStickers.insert(std::make_pair(cube[4][10]->getPermutation(),cube[4][10]->getPermutation()));
	old2NewStickers.insert(std::make_pair(cube[7][4]->getPermutation(),cube[7][4]->getPermutation()));

	//Recreate cube.
	Piece*** cube2 = new Piece**[9];
	for (int i = 0; i < 9; i++) {
		cube2[i] = new Piece*[12];
	}
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 12; j++) {
			cube2[i][j] = new Piece();
		}
	}

	//Use map to find new stickers of scrambled cube.
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 12; j++) {
			auto found = old2NewStickers.find(cube[i][j]->getPermutation());
			if (found != old2NewStickers.end()) {
				cube2[i][j]->setPermutation(found->second);
			}
		}
	}

	//Delete original cube to be replaced with cube2
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 12; j++) {
			delete cube[i][j];
		}
	}
	for (int i = 0; i < 9; i++) {
		delete[] cube[i];
	}
	delete[] cube;

	//Now cube2 with the new values is what "cube" points to
	cube = cube2;
}

std::pair<int,int> Solver::identifyCorner(std::vector<char> givenCorner) {
	std::vector<std::vector<char>> corners = {{'Y', 'O', 'G'}, {'Y', 'R', 'G'},
						  {'Y', 'O', 'B'}, {'Y', 'B', 'R'},
						  {'W', 'O', 'B'}, {'W', 'B', 'R'},
						  {'W', 'O', 'G'}, {'W', 'R', 'G'}};

	int i = 0;
	for (const auto& corner : corners) {
		if (givenCorner == corner) {
			return std::make_pair(i, 0);
		} else if (cornerStickerPerm1(givenCorner) == corner) {
			return std::make_pair(i, 1);
		} else if (cornerStickerPerm2(givenCorner) == corner) {
			return std::make_pair(i, 2);
			//Swaps are not reflexive -> must return inverse order
		} else if (cornerStickerPerm3(givenCorner) == corner) {
			return std::make_pair(i, 4);
			//Swaps are not reflexive -> must return inverse order
		} else if (cornerStickerPerm4(givenCorner) == corner) {
			return std::make_pair(i, 3);
		} else if (cornerStickerPerm5(givenCorner) == corner) {
			return std::make_pair(i, 5);
		}
		i++;
	}

	return std::make_pair(-1, -1);
}

std::pair<int,int> Solver::identifyEdge(std::vector<char> givenEdge) {
	std::vector<std::vector<char>> edges = {{'Y', 'G'}, {'Y', 'O'}, {'Y', 'R'}, {'Y', 'B'},
						{'W', 'B'}, {'W', 'O'}, {'W', 'R'}, {'W', 'G'},
						{'O', 'G'}, {'B', 'O'}, {'R', 'B'}, {'G', 'R'}};

	int i = 0;
	for (const auto& edge : edges) {
		if (givenEdge == edge) {
			return std::make_pair(i, 0);
		} else if (edgeStickerPerm(givenEdge) == edge) {
			return std::make_pair(i, 1);
		}
		i++;
	}

	return std::make_pair(-1, -1);
}

std::vector<char> Solver::cornerStickerPerm1(std::vector<char> vec) {
	std::swap(vec[1], vec[2]);
	return vec;
}

std::vector<char> Solver::cornerStickerPerm2(std::vector<char> vec) {
	std::swap(vec[0], vec[1]);
	return vec;
}

std::vector<char> Solver::cornerStickerPerm3(std::vector<char> vec) {
	std::swap(vec[0], vec[1]);
	std::swap(vec[1], vec[2]);
	return vec;
}

std::vector<char> Solver::cornerStickerPerm4(std::vector<char> vec) {
	std::swap(vec[1], vec[2]);
	std::swap(vec[0], vec[1]);
	return vec;
}

std::vector<char> Solver::cornerStickerPerm5(std::vector<char> vec) {
	std::swap(vec[0], vec[2]);
	return vec;
}

std::vector<std::pair<int,int>> Solver::cornerStickerPermN(std::vector<std::pair<int,int>> vec, int n) {
	switch (n) {
		case 1: {
			std::swap(vec[1], vec[2]);
			break;
		}
		case 2: {
			std::swap(vec[0], vec[1]);
			break;
		}
		case 3: {
			std::swap(vec[0], vec[1]);
			std::swap(vec[1], vec[2]);
			break;
		}
		case 4: {
			std::swap(vec[1], vec[2]);
			std::swap(vec[0], vec[1]);
			break;
		}
		case 5: {
			std::swap(vec[0], vec[2]);
			break;
		}
	}
	return vec;
}

std::vector<char> Solver::edgeStickerPerm(std::vector<char> vec) {
	std::swap(vec[0], vec[1]);
	return vec;
}
std::vector<std::pair<int,int>> Solver::edgeStickerPermN(std::vector<std::pair<int,int>> vec, int n) {
	if (n == 1) {
		std::swap(vec[0], vec[1]);
	}
	return vec;
}

void Solver::simplifyAlgorithm(Queue<std::string>& algorithm) {
	int times = 0, queueSize = algorithm.getLength();
	std::string current = "", next = "", move = "";
	bool simplified = false;

	//Converting from typical Rubik's cube notation to a notation more convenient for the computer (example: F = F1, F2 = F2, F' = F3) and adding the move in the new notation to my simplifier linked list.
	for (int i = 0; i < queueSize; i++) {
		current = algorithm.peekFront();
		if (current.length() == 1) {
			move = current + std::to_string(1);
			simplifier.addBack(move);
		} else if (current[1] == '2') {
			move = current[0] + std::to_string(2);
			simplifier.addBack(move);
		} else if (current[1] == '\'') {
			move = current[0] + std::to_string(3);
			simplifier.addBack(move);
		}
		algorithm.dequeue();
	}

	//std::cout << "Queue length: " << algorithm.getLength() << std::endl;

	std::cout << "\nAlgorithm:\n";
	for (int i = 1; i < simplifier.getLength(); i++) {
		std::cout << simplifier.getEntry(i) << ", ";
	}
	try {
		std::cout << simplifier.getEntry(simplifier.getLength()) << std::endl;
	} catch (PreconditionViolationException& e) {
		std::cout << e.what();
	}

	std::cout << "\nNumber of moves used by program: " << simplifier.getLength() << std::endl;

	do {
		simplified = true;
		for (int i = 1; i < simplifier.getLength() + 1; i++) {
			if ((i != simplifier.getLength()) && (simplifier.getLength() > 1)) {
				current = simplifier.getEntry(i);
				next = simplifier.getEntry(i + 1);
				if (current[0] == next[0]) {
					simplified = false;
					times = (current[1] + next[1]) % 4;
					if (times == 0) {
						try {
							simplifier.removeAt(i);
						} catch (PreconditionViolationException& e) {
							std::cout << e.what();
						}
						try {
							simplifier.removeAt(i);
						} catch (PreconditionViolationException& e) {
							std::cout << e.what();
						}
						i--;
					} else {
						move = current[0] + std::to_string(times);
						try {
							simplifier.removeAt(i);
						} catch (PreconditionViolationException& e) {
							std::cout << e.what();
						}
						try {
							simplifier.removeAt(i);
						} catch (PreconditionViolationException& e) {
							std::cout << e.what();
						}
						try {
							simplifier.insertAt(i, move);
						} catch (PreconditionViolationException& e) {
							std::cout << e.what();
						}
						i--;
					}
				}
			}
		}
	} while (!simplified);

	std::cout << "\nAlgorithm (simplified):\n";
	if (!simplifier.isEmpty()) {
		for (int i = 1; i < simplifier.getLength(); i++) {
			std::cout << simplifier.getEntry(i) << ", ";
		}
		std::cout << simplifier.getEntry(simplifier.getLength()) << std::endl;
	} else {
		std::cout << "No moves!\n";
	}

	for (int i = 1; i < simplifier.getLength() + 1; i++) {
		current = simplifier.getEntry(i);
		if (current[1] == '1') {
			move = current[0];
			algorithm.enqueue(move);
		} else if (current[1] == '2') {
			move = current[0] + std::to_string(2);
			algorithm.enqueue(move);
		} else if (current[1] == '3') {
			//I had to do this, if I just added like in the others, wouldn't work.
			move = "";
			move = move + current[0] + '\'';
			algorithm.enqueue(move);
		}
	}

	while (!simplifier.isEmpty()) {
		simplifier.removeBack();
	}

	//std::cout << "Queue length: " << algorithm.getLength() << std::endl;
}

void Solver::printCube() {
	int perm = 0;
	std::cout << '\n';
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 11; j++) {
			//Separation of rows.
			if ((i % 3 == 0) && (i != 0) && (j == 0)) {
				std::cout << BLUE << "- - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n" << RESET;
						  //<< "- - - - - - - - - - - - - - - - - - - - - - - -\n";
			}

			perm = cube[i][j]->getPermutation();
			if ((perm > 0) && (perm < 10)) {
				std::cout << YELLOW << perm << RESET;
			} else if (((perm > 9) && (perm < 13)) || ((perm > 21) && (perm < 25)) || ((perm > 33) && (perm < 37))) {
				std::cout << MAGENTA << perm << RESET;
			} else if (((perm > 12) && (perm < 16)) || ((perm > 24) && (perm < 28)) || ((perm > 36) && (perm < 40))) {
				std::cout << CYAN << perm << RESET;
			} else if (((perm > 15) && (perm < 19)) || ((perm > 27) && (perm < 31)) || ((perm > 39) && (perm < 43))) {
				std::cout << RED << perm << RESET;
			} else if (((perm > 18) && (perm < 22)) || ((perm > 30) && (perm < 34)) || ((perm > 42) && (perm < 46))) {
				std::cout << GREEN << perm << RESET;
			} else if ((perm > 45) && (perm < 55)) {
				std::cout << WHITE << perm << RESET;
			} else {
				std::cout << " ";
			}

			//WITHOUT COLOR: (fix: print blank instead of zero)
			//std::cout << cube[i][j]->getPermutation() /*<< ", " << cube[i][j]->getOrientation()*/;

			//This fixes the alignment.
			if (cube[i][j]->getPermutation() < 10) {
				std::cout << " ";
			}

			//Separation of columns.
			if (((j % 3 == 2) && (j != 8)) || ((j == 8) && (i > 2) && (i < 6))) {
				std::cout << BLUE << " | " << RESET;
			} else {
				std::cout << "   ";
			}
		}

		perm = cube[i][11]->getPermutation();
		if ((perm > 0) && (perm < 10)) {
			std::cout << YELLOW << perm << RESET << '\n';
		} else if (((perm > 9) && (perm < 13)) || ((perm > 21) && (perm < 25)) || ((perm > 33) && (perm < 37))) {
			std::cout << MAGENTA << perm << RESET << '\n';
		} else if (((perm > 12) && (perm < 16)) || ((perm > 24) && (perm < 28)) || ((perm > 36) && (perm < 40))) {
			std::cout << CYAN << perm << RESET << '\n';
		} else if (((perm > 15) && (perm < 19)) || ((perm > 27) && (perm < 31)) || ((perm > 39) && (perm < 43))) {
			std::cout << RED << perm << RESET << '\n';
		} else if (((perm > 18) && (perm < 22)) || ((perm > 30) && (perm < 34)) || ((perm > 42) && (perm < 46))) {
			std::cout << GREEN << perm << RESET << '\n';
		} else if ((perm > 45) && (perm < 55)) {
			std::cout << WHITE << perm << RESET << '\n';
		} else {
			std::cout << " \n";
		}

		//WITHOUT COLOR: (fix: print blank instead of zero)
		//std::cout << cube[i][11]->getPermutation() /*<< ", " << cube[i][11]->getOrientation()*/ << '\n';
	}
	std::cout << '\n';
}

void Solver::innerMovement(int initialEdgeRow, int initialEdgeCol) {
	int ex = initialEdgeRow, ey = initialEdgeCol, cx = initialEdgeRow, cy = initialEdgeCol - 1, temp = 0;
	//Inner edges:
	temp = cube[ex][ey]->getPermutation();
	cube[ex][ey]->setPermutation(cube[ex + 1][ey - 1]->getPermutation());
	cube[ex + 1][ey - 1]->setPermutation(cube[ex + 2][ey]->getPermutation());
	cube[ex + 2][ey]->setPermutation(cube[ex + 1][ey + 1]->getPermutation());
	cube[ex + 1][ey + 1]->setPermutation(temp);
	//Inner corners:
	temp = cube[cx][cy]->getPermutation();
	cube[cx][cy]->setPermutation(cube[cx + 2][cy]->getPermutation());
	cube[cx + 2][cy]->setPermutation(cube[cx + 2][cy + 2]->getPermutation());
	cube[cx + 2][cy + 2]->setPermutation(cube[cx][cy + 2]->getPermutation());
	cube[cx][cy + 2]->setPermutation(temp);
}

void Solver::F() {
	int temp = 0;
	innerMovement(3,4);
	//Outer edges:
	temp = cube[2][4]->getPermutation();
	cube[2][4]->setPermutation(cube[4][2]->getPermutation());
	cube[4][2]->setPermutation(cube[6][4]->getPermutation());
	cube[6][4]->setPermutation(cube[4][6]->getPermutation());
	cube[4][6]->setPermutation(temp);
	//Left outer corners:
	temp = cube[3][2]->getPermutation();
	cube[3][2]->setPermutation(cube[6][3]->getPermutation());
	cube[6][3]->setPermutation(cube[5][6]->getPermutation());
	cube[5][6]->setPermutation(cube[2][5]->getPermutation());
	cube[2][5]->setPermutation(temp);
	//Right outer corners:
	temp = cube[5][2]->getPermutation();
	cube[5][2]->setPermutation(cube[6][5]->getPermutation());
	cube[6][5]->setPermutation(cube[3][6]->getPermutation());
	cube[3][6]->setPermutation(cube[2][3]->getPermutation());
	cube[2][3]->setPermutation(temp);
	solverAlg.enqueue("F");
}

void Solver::R() {
	int temp = 0;
	innerMovement(3,7);
	//Outer edges:
	temp = cube[1][5]->getPermutation();
	cube[1][5]->setPermutation(cube[4][5]->getPermutation());
	cube[4][5]->setPermutation(cube[7][5]->getPermutation());
	cube[7][5]->setPermutation(cube[4][9]->getPermutation());
	cube[4][9]->setPermutation(temp);
	//Left outer corners:
	temp = cube[3][5]->getPermutation();
	cube[3][5]->setPermutation(cube[6][5]->getPermutation());
	cube[6][5]->setPermutation(cube[5][9]->getPermutation());
	cube[5][9]->setPermutation(cube[0][5]->getPermutation());
	cube[0][5]->setPermutation(temp);
	//Right outer corners:
	temp = cube[5][5]->getPermutation();
	cube[5][5]->setPermutation(cube[8][5]->getPermutation());
	cube[8][5]->setPermutation(cube[3][9]->getPermutation());
	cube[3][9]->setPermutation(cube[2][5]->getPermutation());
	cube[2][5]->setPermutation(temp);
	solverAlg.enqueue("R");
}

void Solver::U() {
	int temp = 0;
	innerMovement(0,4);
	//Outer edges:
	temp = cube[3][10]->getPermutation();
	cube[3][10]->setPermutation(cube[3][1]->getPermutation());
	cube[3][1]->setPermutation(cube[3][4]->getPermutation());
	cube[3][4]->setPermutation(cube[3][7]->getPermutation());
	cube[3][7]->setPermutation(temp);
	//Left outer corners:
	temp = cube[3][0]->getPermutation();
	cube[3][0]->setPermutation(cube[3][3]->getPermutation());
	cube[3][3]->setPermutation(cube[3][6]->getPermutation());
	cube[3][6]->setPermutation(cube[3][9]->getPermutation());
	cube[3][9]->setPermutation(temp);
	//Right outer corners:
	temp = cube[3][2]->getPermutation();
	cube[3][2]->setPermutation(cube[3][5]->getPermutation());
	cube[3][5]->setPermutation(cube[3][8]->getPermutation());
	cube[3][8]->setPermutation(cube[3][11]->getPermutation());
	cube[3][11]->setPermutation(temp);
	solverAlg.enqueue("U");
}

void Solver::D() {
	int temp = 0;
	innerMovement(6,4);
	//Outer edges:
	temp = cube[5][4]->getPermutation();
	cube[5][4]->setPermutation(cube[5][1]->getPermutation());
	cube[5][1]->setPermutation(cube[5][10]->getPermutation());
	cube[5][10]->setPermutation(cube[5][7]->getPermutation());
	cube[5][7]->setPermutation(temp);
	//Left outer corners:
	temp = cube[5][2]->getPermutation();
	cube[5][2]->setPermutation(cube[5][11]->getPermutation());
	cube[5][11]->setPermutation(cube[5][8]->getPermutation());
	cube[5][8]->setPermutation(cube[5][5]->getPermutation());
	cube[5][5]->setPermutation(temp);
	//Right outer corners:
	temp = cube[5][0]->getPermutation();
	cube[5][0]->setPermutation(cube[5][9]->getPermutation());
	cube[5][9]->setPermutation(cube[5][6]->getPermutation());
	cube[5][6]->setPermutation(cube[5][3]->getPermutation());
	cube[5][3]->setPermutation(temp);
	solverAlg.enqueue("D");
}

void Solver::L() {
	int temp = 0;
	innerMovement(3,1);
	//Outer edges:
	temp = cube[1][3]->getPermutation();
	cube[1][3]->setPermutation(cube[4][11]->getPermutation());
	cube[4][11]->setPermutation(cube[7][3]->getPermutation());
	cube[7][3]->setPermutation(cube[4][3]->getPermutation());
	cube[4][3]->setPermutation(temp);
	//Left outer corners:
	temp = cube[3][11]->getPermutation();
	cube[3][11]->setPermutation(cube[8][3]->getPermutation());
	cube[8][3]->setPermutation(cube[5][3]->getPermutation());
	cube[5][3]->setPermutation(cube[2][3]->getPermutation());
	cube[2][3]->setPermutation(temp);
	//Right outer corners:
	temp = cube[3][3]->getPermutation();
	cube[3][3]->setPermutation(cube[0][3]->getPermutation());
	cube[0][3]->setPermutation(cube[5][11]->getPermutation());
	cube[5][11]->setPermutation(cube[6][3]->getPermutation());
	cube[6][3]->setPermutation(temp);
	solverAlg.enqueue("L");
}

void Solver::B() {
	int temp = 0;
	innerMovement(3,10);
	//Outer edges:
	temp = cube[0][4]->getPermutation();
	cube[0][4]->setPermutation(cube[4][8]->getPermutation());
	cube[4][8]->setPermutation(cube[8][4]->getPermutation());
	cube[8][4]->setPermutation(cube[4][0]->getPermutation());
	cube[4][0]->setPermutation(temp);
	//Left outer corners:
	temp = cube[3][8]->getPermutation();
	cube[3][8]->setPermutation(cube[8][5]->getPermutation());
	cube[8][5]->setPermutation(cube[5][0]->getPermutation());
	cube[5][0]->setPermutation(cube[0][3]->getPermutation());
	cube[0][3]->setPermutation(temp);
	//Right outer corners:
	temp = cube[3][0]->getPermutation();
	cube[3][0]->setPermutation(cube[0][5]->getPermutation());
	cube[0][5]->setPermutation(cube[5][8]->getPermutation());
	cube[5][8]->setPermutation(cube[8][3]->getPermutation());
	cube[8][3]->setPermutation(temp);
	solverAlg.enqueue("B");
}

void Solver::E() {
	int temp = 0;
	//Left Edges:
	temp = cube[4][3]->getPermutation();
	cube[4][3]->setPermutation(cube[4][0]->getPermutation());
	cube[4][0]->setPermutation(cube[4][9]->getPermutation());
	cube[4][9]->setPermutation(cube[4][6]->getPermutation());
	cube[4][6]->setPermutation(temp);
	//Centers:
	temp = cube[4][4]->getPermutation();
	cube[4][4]->setPermutation(cube[4][1]->getPermutation());
	cube[4][1]->setPermutation(cube[4][10]->getPermutation());
	cube[4][10]->setPermutation(cube[4][7]->getPermutation());
	cube[4][7]->setPermutation(temp);
	//Right Edges:
	temp = cube [4][5]->getPermutation();
	cube[4][5]->setPermutation(cube[4][2]->getPermutation());
	cube[4][2]->setPermutation(cube[4][11]->getPermutation());
	cube[4][11]->setPermutation(cube[4][8]->getPermutation());
	cube[4][8]->setPermutation(temp);
	solverAlg.enqueue("E");
}

void Solver::Fp() {
	for (int i = 0; i < 3; i++) {
		F();
	}
}

void Solver::Rp() {
	for (int i = 0; i < 3; i++) {
		R();
	}
}

void Solver::Up() {
	for (int i = 0; i < 3; i++) {
		U();
	}
}

void Solver::Dp() {
	for (int i = 0; i < 3; i++) {
		D();
	}
}

void Solver::Lp() {
	for (int i = 0; i < 3; i++) {
		L();
	}
}

void Solver::Bp() {
	for (int i = 0; i < 3; i++) {
		B();
	}
}

void Solver::Ep() {
	for (int i = 0; i < 3; i++) {
		E();
	}
}

void Solver::F2() {
	for (int i = 0; i < 2; i++) {
		F();
	}
}

void Solver::R2() {
	for (int i = 0; i < 2; i++) {
		R();
	}
}

void Solver::U2() {
	for (int i = 0; i < 2; i++) {
		U();
	}
}

void Solver::D2() {
	for (int i = 0; i < 2; i++) {
		D();
	}
}

void Solver::L2() {
	for (int i = 0; i < 2; i++) {
		L();
	}
}

void Solver::B2() {
	for (int i = 0; i < 2; i++) {
		B();
	}
}

void Solver::E2() {
	for (int i = 0; i < 2; i++) {
		E();
	}
}

void Solver::Cross() {
	//FIRST EDGE:
	bool edgeOriented = false;

	if (foundInferiorEdge('D', 47) || foundInferiorEdge('U', 47) || foundInferiorEdge('F', 47) ||
		foundInferiorEdge('R', 47) || foundInferiorEdge('B', 47) || foundInferiorEdge('L', 47)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "First edge oriented!\n\n";
	}

	//SECOND EDGE:
	Dp();
	edgeOriented = false;
	if (foundInferiorEdge('D', 51) || foundInferiorEdge('U', 51) || foundInferiorEdge('F', 51) ||
		foundInferiorEdge('R', 51) || foundInferiorEdge('B', 51) || foundInferiorEdge('L', 51)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "Second edge oriented!\n\n";
	}

	//THIRD EDGE:
	Dp();
	edgeOriented = false;
	if (foundInferiorEdge('D', 53) || foundInferiorEdge('U', 53) || foundInferiorEdge('F', 53) ||
		foundInferiorEdge('R', 53) || foundInferiorEdge('B', 53) || foundInferiorEdge('L', 53)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "Third edge oriented!\n\n";
	}

	//FOURTH EDGE:
	Dp();
	edgeOriented = false;
	if (foundInferiorEdge('D', 49) || foundInferiorEdge('U', 49) || foundInferiorEdge('F', 49) ||
		foundInferiorEdge('R', 49) || foundInferiorEdge('B', 49) || foundInferiorEdge('L', 49)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "Fourth edge oriented!\n\n";
	}

	Dp();
	printCube();
	std::cout << "Bottom cross solved!\n\n";
}

void Solver::FirstLayerF2L() {
	//FIRST CORNER:
	bool cornerOriented = false;
	if (foundInferiorCorner('D', 48) || foundInferiorCorner('U', 48) || foundInferiorCorner('F', 48) ||
		foundInferiorCorner('R', 48) || foundInferiorCorner('B', 48) || foundInferiorCorner('L', 48)) {
		cornerOriented = true;
	}
	if (cornerOriented) {
		std::cout << "First corner oriented!\n\n";
	}

	//SECOND CORNER:
	Dp();
	cornerOriented = false;
	if (foundInferiorCorner('D', 54) || foundInferiorCorner('U', 54) || foundInferiorCorner('F', 54) ||
		foundInferiorCorner('R', 54) || foundInferiorCorner('B', 54) || foundInferiorCorner('L', 54)) {
		cornerOriented = true;
	}
	if (cornerOriented) {
		std::cout << "Second corner oriented!\n\n";
	}

	//THIRD CORNER:
	Dp();
	cornerOriented = false;
	if (foundInferiorCorner('D', 52) || foundInferiorCorner('U', 52) || foundInferiorCorner('F', 52) ||
		foundInferiorCorner('R', 52) || foundInferiorCorner('B', 52) || foundInferiorCorner('L', 52)) {
		cornerOriented = true;
	}
	if (cornerOriented) {
		std::cout << "Third corner oriented!\n\n";
	}

	//FOURTH CORNER:
	Dp();
	cornerOriented = false;
	if (foundInferiorCorner('D', 46) || foundInferiorCorner('U', 46) || foundInferiorCorner('F', 46) ||
		foundInferiorCorner('R', 46) || foundInferiorCorner('B', 46) || foundInferiorCorner('L', 46)) {
		cornerOriented = true;
	}
	if (cornerOriented) {
		std::cout << "Fourth corner oriented!\n\n";
	}

	Dp();
	printCube();
	std::cout << "Bottom corners solved!\n\n";
}

void Solver::SecondLayerF2L() {
	//FIRST EDGE:
	bool edgeOriented = false;
	if (foundMiddleEdge('U', 27) || foundMiddleEdge('F', 27) || foundMiddleEdge('R', 27) ||
		foundMiddleEdge('B', 27) || foundMiddleEdge('L', 27)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "First edge oriented!\n\n";
	}

	//SECOND EDGE:
	Ep();
	edgeOriented = false;
	if (foundMiddleEdge('U', 30) || foundMiddleEdge('F', 30) || foundMiddleEdge('R', 30) ||
		foundMiddleEdge('B', 30) || foundMiddleEdge('L', 30)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "Second edge oriented!\n\n";
	}

	//THIRD EDGE:
	Ep();
	edgeOriented = false;
	if (foundMiddleEdge('U', 33) || foundMiddleEdge('F', 33) || foundMiddleEdge('R', 33) ||
		foundMiddleEdge('B', 33) || foundMiddleEdge('L', 33)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "Third edge oriented!\n\n";
	}

	//FOURTH EDGE:
	Ep();
	edgeOriented = false;
	if (foundMiddleEdge('U', 24) || foundMiddleEdge('F', 24) || foundMiddleEdge('R', 24) ||
		foundMiddleEdge('B', 24) || foundMiddleEdge('L', 24)) {
		edgeOriented = true;
	}
	if (edgeOriented) {
		std::cout << "Fourth edge oriented!\n\n";
	}

	Ep();
	printCube();
	std::cout << "Middle edges solved!\n\n";
}

void Solver::EdgesOLL() {
	bool edgesOriented = false;
	if (orientedTopEdges(2, 6, 8, 4)) {
		edgesOriented = true;
	}
	if (edgesOriented) {
		std::cout << "Top edges oriented!\n\n";
	}
}

void Solver::EdgesPLL() {
	bool edgesPermuted = false;
	if (permutedTopEdges(14, 17, 20, 11)) {
		edgesPermuted = true;
	}
	if (edgesPermuted) {
		std::cout << "Top edges permuted!\n\n";
	}
}

void Solver::CornersPLL() {
	bool cornersPermuted = false;
	if (permutedTopCorners(9, 3, 1, 7)) {
		cornersPermuted= true;
	}
	if (cornersPermuted) {
		std::cout << "Top corners permuted!\n\n";
	}
}

void Solver::CornersOLL() {
	bool cornersOriented = false;
	if (orientedTopCorners(9, 3, 1, 7)) {
		cornersOriented = true;
	}
	if (cornersOriented) {
		std::cout << "Top corners oriented!\n\n";
	}
}

bool Solver::foundInferiorEdge(char face, int edge) {
	int x = 0, y = 0, e = edge, option = 0;
	if (face == 'D') {
		x = 6;
		y = 3;
		option = 1;
	} else if (face == 'U') {
		x = 0;
		y = 3;
		option = 2;
	} else if (face == 'F') {
		x = 3;
		y = 3;
		option = 3;
	} else if (face == 'R') {
		x = 3;
		y = 6;
		option = 4;
	} else if (face == 'B') {
		x = 3;
		y = 9;
		option = 5;
	} else if (face == 'L') {
		x = 3;
		y = 0;
		option = 6;
	}
	//Find the first piece and according to here it is, perform a certain userAlgorithm.
	for (int i = x; i < x + 3; i++) {
		for (int j = y; j < y + 3; j++) {
			if(cube[i][j]->getPermutation() == e) {

				if ((i == x) && (j == y + 1)) {
					//std::cout << "Found in " << face << " top.\n";
					switch (option) {
						case 2:
							U2();
							F2();
							break;
						case 3:
							flipEdge();
							break;
						case 4:
							U();
							flipEdge();
							break;
						case 5:
							U2();
							flipEdge();
							break;
						case 6:
							Up();
							flipEdge();
							break;
					}

				} else if ((i == x + 1) && (j == y + 2)) {
					//std::cout << "Found in " << face << " right.\n";
					switch (option) {
						case 1:
							R2();
							U();
							F2();
							break;
						case 2:
							U();
							F2();
							break;
						case 3:
							Fp();
							flipEdge();
							break;
						case 4:
							Rp();
							U();
							R();
							flipEdge();
							break;
						case 5:
							Bp();
							U2();
							B();
							flipEdge();
							break;
						case 6:
							Lp();
							Up();
							L();
							flipEdge();
							break;
					}

				} else if ((i == x + 2) && (j == y + 1)) {
					//std::cout << "Found in " << face << " bottom.\n";
					switch (option) {
						case 1:
							B2();
							U2();
							F2();
							break;
						case 2:
							F2();
							break;
						case 3:
							F2();
							flipEdge();
							break;
						case 4:
							R2();
							U();
							flipEdge();
							break;
						case 5:
							B2();
							U2();
							flipEdge();
							break;
						case 6:
							L2();
							Up();
							flipEdge();
							break;
					}

				} else {
					//std::cout << "Found in " << face << " left.\n";
					switch (option) {
						case 1:
							L2();
							Up();
							F2();
							break;
						case 2:
							Up();
							F2();
							break;
						case 3:
							F();
							flipEdge();
							break;
						case 4:
							R();
							U();
							Rp();
							flipEdge();
							break;
						case 5:
							B();
							U2();
							Bp();
							flipEdge();
							break;
						case 6:
							L();
							Up();
							Lp();
							flipEdge();
							break;
					}
				}

				std::cout << "\n\n";
				printCube();
				return (true);
			}
		}
	}
	return (false);
}

bool Solver::foundInferiorCorner(char face, int corner) {
	int x = 0, y = 0, c = corner, option = 0;
	if (face == 'D') {
		x = 6;
		y = 3;
		option = 1;
	} else if (face == 'U') {
		x = 0;
		y = 3;
		option = 2;
	} else if (face == 'F') {
		x = 3;
		y = 3;
		option = 3;
	} else if (face == 'R') {
		x = 3;
		y = 6;
		option = 4;
	} else if (face == 'B') {
		x = 3;
		y = 9;
		option = 5;
	} else if (face == 'L') {
		x = 3;
		y = 0;
		option = 6;
	}
	//Find the first piece and according to where it is, perform a certain userAlgorithm.
	for (int i = x; i < x + 3; i++) {
		for (int j = y; j < y + 3; j++) {
			if(cube[i][j]->getPermutation() == c) {

				if ((i == x) && (j == y)) {
					//std::cout << "Found in " << face << " left superior.\n";
					switch (option) {
						case 1:
							D();
							removeCornerUp();
							Dp();
							insertRightCorner();
							break;
						case 2:
							U2();
							insertTopCorner();
							break;
						case 3:
							Up();
							insertRightCorner();
							break;
						case 4:
							insertRightCorner();
							break;
						case 5:
							U();
							insertRightCorner();
							break;
						case 6:
							U2();
							insertRightCorner();
							break;
					}

				} else if ((i == x) && (j == y + 2)) {
					//std::cout << "Found in " << face << " right superior.\n";
					switch (option) {
						case 2:
							U();
							insertTopCorner();
							break;
						case 3:
							insertLeftCorner();
							break;
						case 4:
							U();
							insertLeftCorner();
							break;
						case 5:
							U2();
							insertLeftCorner();
							break;
						case 6:
							Up();
							insertLeftCorner();
							break;
					}

				} else if ((i == x + 2) && (j == y + 2)) {
					//std::cout << "Found in " << face << " right inferior.\n";
					switch (option) {
						case 1:
							Dp();
							removeCornerUp();
							D();
							insertRightCorner();
							break;
						case 2:
							insertTopCorner();
							break;
						case 3:
							removeCornerUp();
							insertLeftCorner();
							break;
						case 4:
							Dp();
							removeCornerUp();
							D();
							insertLeftCorner();
							break;
						case 5:
							D2();
							removeCornerUp();
							D2();
							insertLeftCorner();
							break;
						case 6:
							D();
							removeCornerUp();
							Dp();
							insertLeftCorner();
							break;
					}

				} else {
					//std::cout << "Found in " << face << " left inferior.\n";
					switch (option) {
						case 1:
							D2();
							removeCornerUp();
							D2();
							insertRightCorner();
							break;
						case 2:
							Up();
							insertTopCorner();
							break;
						case 3:
							D();
							removeCornerU();
							Dp();
							Up();
							insertRightCorner();
							break;
						case 4:
							removeCornerU();
							Up();
							insertRightCorner();
							break;
						case 5:
							Dp();
							removeCornerU();
							D();
							Up();
							insertRightCorner();
							break;
						case 6:
							D2();
							removeCornerU();
							D2();
							Up();
							insertRightCorner();
							break;
					}
				}
				std::cout << "\n\n";
				printCube();
				return (true);
			}
		}
	}
	return (false);
}

bool Solver::foundMiddleEdge(char face, int edge) {
	//This time, we don't care to look in face D.
	int x = 0, y = 0, e = edge, option = 0;
	if (face == 'U') {
		x = 0;
		y = 3;
		option = 1;
	} else if (face == 'F') {
		x = 3;
		y = 3;
		option = 2;
	} else if (face == 'R') {
		x = 3;
		y = 6;
		option = 3;
	} else if (face == 'B') {
		x = 3;
		y = 9;
		option = 4;
	} else if (face == 'L') {
		x = 3;
		y = 0;
		option = 5;
	}
	//Find the first piece and according to here it is, perform a certain userAlgorithm.
	for (int i = x; i < x + 3; i++) {
		for (int j = y; j < y + 3; j++) {
			if(cube[i][j]->getPermutation() == e) {

				if ((i == x) && (j == y + 1)) {
					//std::cout << "Found in " << face << " top.\n";
					switch (option) {
						case 1:
							U();
							insertAtLeft();
							break;
						case 2:
							insertAtRight();
							break;
						case 3:
							U();
							insertAtRight();
							break;
						case 4:
							U2();
							insertAtRight();
							break;
						case 5:
							Up();
							insertAtRight();
							break;
					}

				} else if ((i == x + 1) && (j == y + 2)) {
					//std::cout << "Found in " << face << " right.\n";
					switch (option) {
						case 1:
							insertAtLeft();
							break;
						case 3:
							Ep();
							removeAtRight();
							E();
							U();
							insertAtLeft();
							break;
						case 4:
							E2();
							removeAtLeft();
							E2();
							Up();
							insertAtRight();
							break;
						case 5:
							E();
							removeAtLeft();
							Ep();
							Up();
							insertAtRight();
							break;
					}

				} else if ((i == x + 2) && (j == y + 1)) {
					//std::cout << "Found in " << face << " bottom.\n";
					switch (option) {
						case 1:
							Up();
							insertAtLeft();
							break;
					}
				} else {
					//std::cout << "Found in " << face << " left.\n";
					switch (option) {
						case 1:
							U2();
							insertAtLeft();
							break;
						case 2:
							E();
							removeAtRight();
							Ep();
							U2();
							insertAtRight();
							break;
						case 3:
							removeAtLeft();
							U2();
							insertAtLeft();
							break;
						case 4:
							Ep();
							removeAtRight();
							E();
							U2();
							insertAtRight();
							break;
						case 5:
							E2();
							removeAtRight();
							E2();
							U2();
							insertAtRight();
							break;
					}
				}

				std::cout << "\n\n";
				printCube();
				return (true);
			}
		}
	}
	return (false);
}

bool Solver::orientedTopEdges(int edge1, int edge2, int edge3, int edge4) {
	int e1 = edge1, e2 = edge2, e3 = edge3, e4 = edge4, counter = 0;

	//Making an array of coordinates.
	int* coordinates = new int[8];
	for (int i = 0; i < 8; i++) {
		coordinates[i] = 0;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 3; j < 6; j++) {
			if ((cube[i][j]->getPermutation() == e1) || (cube[i][j]->getPermutation() == e2) ||
				(cube[i][j]->getPermutation() == e3) || (cube[i][j]->getPermutation() == e4)) {
				coordinates[counter] = i;
				counter++;
				coordinates[counter] = j;
				counter++;
			}
		}
	}

	if (counter == 0) {
		FRURUF();
		U2();
		FRURUF();
		FRURUF();
		std::cout << "\n\n";
		printCube();
		return(true);
	} else if (counter == 4) {
		int amountOf1s = 0, amountof4s = 0;
		for (int i = 0; i < 8; i++) {
			if (coordinates[i] == 1) {
				amountOf1s++;
			} else if (coordinates[i] == 4) {
				amountof4s++;
			}
		}

		//Horizontal Line:
		if (amountOf1s == 2) {
			FRURUF();
			std::cout << "\n\n";
			printCube();
			return (true);
		}

		//Vertical Line:
		if (amountof4s == 2) {
			U();
			FRURUF();
			std::cout << "\n\n";
			printCube();
			return (true);
		}

		int sum = 0, largest = 0;
		for (int i = 0; i < 8; i++) {
			sum += coordinates[i];
			if (coordinates[i] > largest) {
				largest = coordinates[i];
			}
		}

		//Right Superior L:
		if ((sum == 10) && (largest == 5)) {
			Up();
			FRURUF();
			FRURUF();
		//Right Inferior L:
		} else if ((sum == 12) && (largest == 5)) {
			U2();
			FRURUF();
			FRURUF();
		//Left Inferior L:
		} else if ((sum == 10) && (largest == 4)) {
			U();
			FRURUF();
			FRURUF();
		//Left Superior L:
		} else if ((sum == 8) && (largest == 4)) {
			FRURUF();
			FRURUF();
		}
		std::cout << "\n\n";
		printCube();
		return(true);
	} else if (counter == 8) {
		std::cout << "\n\n";
		printCube();
		return(true);
	}

	delete[] coordinates;
	return (false);
}

bool Solver::permutedTopEdges(int edge1, int edge2, int edge3, int edge4) {
	int e1 = edge1, e2 = edge2, e3 = edge3, e4 = edge4, position_e1 = 0;
	bool foundRight = false, foundUp = false, foundLeft = false, clockwiseFix = false;

	for (int j = 0; j < 12; j++) {
		if (cube[3][j]->getPermutation() == e1) {
			position_e1 = j;
			if (cube[3][(j + 3) % 12]->getPermutation() == e2) {
				foundRight = true;
			}
			if (cube[3][(j + 6) % 12]->getPermutation() == e3) {
				foundUp = true;
			}
			if (cube[3][(j + 9) % 12]->getPermutation() == e4) {
				foundLeft = true;
			}
		}
	}

	if (cube[3][(position_e1 + 6) % 12]->getPermutation() == e2) {
		clockwiseFix = true;
	} else if (cube[3][(position_e1 + 6) % 12]->getPermutation() == e4) {
		clockwiseFix = false;
	}

	if (position_e1 == 1) {
		Up();
	} else if (position_e1 == 7) {
		U();
	} else if (position_e1 == 10) {
		U2();
	}

	if (foundRight && foundUp && foundLeft) {
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if (!foundRight && foundUp && !foundLeft) {
		train();
		Up();
		train();
		U2();
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if (foundRight && !foundUp && !foundLeft) {
		Up();
		train();
		U2();
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if (!foundRight && !foundUp && foundLeft) {
		U2();
		train();
		Up();
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if (!foundRight && !foundUp && !foundLeft) {
		if (clockwiseFix) {
			train();
			train();
		} else {
			train();
		}
		std::cout << "\n\n";
		printCube();
		return(true);
	}

	return (false);
}

bool Solver::permutedTopCorners(int corner1, int corner2, int corner3, int corner4) {
	int c1 = corner1, c2 = corner2, c3 = corner3, c4 = corner4;
	bool clockwiseFix = false;
	if ((cube[2][5]->getPermutation() == c1) || (cube[3][5]->getPermutation() == c1) || (cube[3][6]->getPermutation() == c1)) {
		if ((cube[0][3]->getPermutation() == c2) || (cube[3][11]->getPermutation() == c2) || (cube[3][0]->getPermutation() == c2)) {
			clockwiseFix = true;
		} else if ((cube[0][3]->getPermutation() == c4) || (cube[3][11]->getPermutation() == c4) || (cube[3][0]->getPermutation() == c4)) {
			clockwiseFix = false;
		} else {
			std::cout << "\n\n";
			printCube();
			return (true);
		}
		U2();
		if (!clockwiseFix) {
			inAndOut();
		}
		inAndOut();
		U2();
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if ((cube[0][5]->getPermutation() == c2) || (cube[3][8]->getPermutation() == c2) || (cube[3][9]->getPermutation() == c2)) {
		if ((cube[2][3]->getPermutation() == c3) || (cube[3][2]->getPermutation() == c3) || (cube[3][3]->getPermutation() == c3)) {
			clockwiseFix = true;
		} else if ((cube[2][3]->getPermutation() == c1) || (cube[3][2]->getPermutation() == c1) || (cube[3][3]->getPermutation() == c1)) {
			clockwiseFix = false;
		} else {
			std::cout << "\n\n";
			printCube();
			return (true);
		}
		Up();
		if (!clockwiseFix) {
			inAndOut();
		}
		inAndOut();
		U();
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if ((cube[0][3]->getPermutation() == c3) || (cube[3][11]->getPermutation() == c3) || (cube[3][0]->getPermutation() == c3)) {
		if ((cube[2][5]->getPermutation() == c4) || (cube[3][5]->getPermutation() == c4) || (cube[3][6]->getPermutation() == c4)) {
			clockwiseFix = true;
		} else if ((cube[2][5]->getPermutation() == c2) || (cube[3][5]->getPermutation() == c2) || (cube[3][6]->getPermutation() == c2)) {
			clockwiseFix = false;
		} else {
			std::cout << "\n\n";
			printCube();
			return (true);
		}
		if (!clockwiseFix) {
			inAndOut();
		}
		inAndOut();
		std::cout << "\n\n";
		printCube();
		return(true);

	} else if ((cube[2][3]->getPermutation() == c4) || (cube[3][2]->getPermutation() == c4) || (cube[3][3]->getPermutation() == c4)) {
		if ((cube[0][5]->getPermutation() == c1) || (cube[3][8]->getPermutation() == c1) || (cube[3][9]->getPermutation() == c1)) {
			clockwiseFix = true;
		} else if ((cube[0][5]->getPermutation() == c3) || (cube[3][8]->getPermutation() == c3) || (cube[3][9]->getPermutation() == c3)) {
			clockwiseFix = false;
		} else {
			std::cout << "\n\n";
			printCube();
			return (true);
		}
		U();
		if (!clockwiseFix) {
			inAndOut();
		}
		inAndOut();
		Up();
		std::cout << "\n\n";
		printCube();
		return(true);
	} else {
		inAndOut();
		return(permutedTopCorners(9, 3, 1, 7));
	}

	return(false);
}

bool Solver::orientedTopCorners(int corner1, int corner2, int corner3, int corner4) {
	int c1 = corner1, c2 = corner2, c3 = corner3, c4 = corner4;
	if (cube[2][5]->getPermutation() == c1) {
		U();
	} else {
		while (cube[2][5]->getPermutation() != c1) {
			RDRD();
		}
		U();
	}

	if (cube[2][5]->getPermutation() == c2) {
		U();
	} else {
		while (cube[2][5]->getPermutation() != c2) {
			RDRD();
		}
		U();
	}

	if (cube[2][5]->getPermutation() == c3) {
		U();
	} else {
		while (cube[2][5]->getPermutation() != c3) {
			RDRD();
		}
		U();
	}

	if (cube[2][5]->getPermutation() == c4) {
		U();
	} else {
		while (cube[2][5]->getPermutation() != c4) {
			RDRD();
		}
		U();
	}

	std::cout << "\n\n";
	printCube();
	return(true);
}

//userAlgorithms
void Solver::flipEdge() {
	U(); L(); Fp(); Lp();
}

void Solver::insertRightCorner() {
	R(); U(); Rp();
}

void Solver::insertLeftCorner() {
	Fp(); Up(); F();
}

void Solver::removeCornerU() {
	R(); U(); Rp();
}


void Solver::removeCornerUp() {
	R(); Up(); Rp();
}

void Solver::insertTopCorner() {
	R(); U2(); Rp(); Up(); insertRightCorner();
}

void Solver::insertAtLeft() {
	Up(); Fp(); U(); F(); Rp(); F(); R(); Fp();
}

void Solver::insertAtRight() {
	U(); R(); Up(); Rp(); F(); Rp(); Fp(); R();
}

void Solver::removeAtLeft() {
	Up(); Fp(); U(); F(); Rp(); F(); R(); Fp();
}

void Solver::removeAtRight() {
	U(); R(); Up(); Rp(); F(); Rp(); Fp(); R();
}

void Solver::FRURUF() {
	F(); R(); U(); Rp(); Up(); Fp();
}

void Solver::train() {
	R(); U(); Rp(); U(); R(); U2(); Rp();
}

void Solver::inAndOut() {
	Rp(); U(); L(); Up(); R(); U(); Lp(); Up();
}

void Solver::RDRD() {
	Rp(); Dp(); R(); D();
}
