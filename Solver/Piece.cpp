#include "Piece.h"

Piece::Piece() {
	p_permutation = 0;
}

int Piece::getPermutation() const {
	return(p_permutation);
}

void Piece::setPermutation(int permutation) {
	p_permutation = permutation;
}
