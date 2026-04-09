//
// Created by clair on 4/9/2026.
//

#ifndef CHESS22L_SQUARE_H
#define CHESS22L_SQUARE_H
typedef enum file {
	A, B, C, D, E, F, G, H, I, J
} file;
typedef struct Square {
	file file;
	int rank;
} Square;

#endif //CHESS22L_SQUARE_H
