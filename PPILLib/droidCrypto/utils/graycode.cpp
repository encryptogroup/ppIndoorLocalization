/******************************************************************************
 *
 *                 M4RI: Linear Algebra over GF(2)
 *
 *    Copyright (C) 2007 Gregory Bard <gregory.bard@ieee.org>
 *    Copyright (C) 2007 Martin Albrecht <malb@informatik.uni-bremen.de>
 *
 *  Distributed under the terms of the GNU General Public License (GPL)
 *  version 2 or higher.
 *
 *    This code is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *  The full text of the GPL is available at:
 *
 *                  http://www.gnu.org/licenses/
 ******************************************************************************/

#include "graycode.h"
#include <cstdlib>
#include <cstdint>

#define two_pow(e) (((uint64_t) 1) << (e))

int gray_code(int number, int length) {
	int lastbit = 0;
	int res = 0;
	for (int i = length - 1; i >= 0; --i) {
		int bit = number & (1 << i);
		res |= (lastbit >> 1) ^ bit;
		lastbit = bit;
	}
	return res;
}

code* build_code(int l) {
	code* codebook = new code;

	codebook->ord = new int[two_pow(l)];
	codebook->inc = new int[two_pow(l)];

	for (int i = 0; i < (int) two_pow(l); ++i) {
		codebook->ord[i] = gray_code(i, l);
	}

	for (int i = l; i > 0; --i) {
		for (int j = 1; j < (int) two_pow(i) + 1; ++j) {
			codebook->inc[j * two_pow(l - i) - 1] = l - i;
		}
	}

	return codebook;
}

void destroy_code(code* codebook) {
	if (!codebook) {
		return;
	}
	delete[] codebook->inc;
	delete[] codebook->ord;
	delete codebook;
}
