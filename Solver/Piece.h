#ifndef PIECE_H
#define PIECE_H

#include <string>

class Piece {
	public:
		Piece();
		int getPermutation() const;
		void setPermutation(int permutation);
	protected:
		int p_permutation;
};

#endif
