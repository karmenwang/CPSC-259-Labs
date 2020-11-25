/*
FILE: takehomelab5.c
Purpose: computes the page ranks of a web with hyperlinked paged using matlab
Author: Karmen Wang and Matthew Lee
Student #s:	 	54144183 and 78927936
CS Accounts:	    t6k1b and z9u1b
Date: Nov 26 2018
*/


#define FILE_NAME "web.txt"
#define BUFFERLINESIZE 1234
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"

int get_maze_dimension(FILE*);
int** parse_maze(FILE*, int);


int main(void) {

	//Variables & Matlab Variables
	int i;
	int error = 0;
	char line_buffer[BUFFERLINESIZE+1];
	int** connectivity_matrix = NULL;
	FILE * file_pointer = NULL;
	int dimension = 0;
	Engine *ep = NULL; // A pointer to a MATLAB engine object
	mxArray *connectivity = NULL, *pagerank = NULL, *dimentions = NULL; // mxArray is the fundamental type underlying MATLAB data


//Step 1: check if text file is valid and copy into 2d dynamic memory
	error = fopen_s(&file_pointer, FILE_NAME, "r");
	if (error) {
		fprintf(stderr, "Unable to open file: %s\n", "web.txt");
		system("pause");
		return 1;
	}

	//if file is valid...
	if(file_pointer) {
		dimension = get_maze_dimension(file_pointer);
		//check if dimensions are at least 2 or else will not work
		if (dimension < 2) {
			printf("Dimension size is invalid, must be at least size 2\n");
			system("pause");
			return 1;
		}
		
		//get connectivity matrix
		connectivity_matrix = parse_maze(file_pointer, dimension);
	}
	//taken from lab 4
	else {
		fprintf(stderr, "Unable to parse web: %s\n", FILE_NAME);
		system("pause");
		return 1;
	}

//Step 2: Put connectivity matrix into matlab and calculate pagerank
	/*start Matlab Process*/

	// if statement taken from lab4
	if (!(ep = engOpen(NULL))) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		system("pause");
		return 1;
	}

	//put connectivity matrix into matlab
	connectivity_matrix = mxCreateDoubleMatrix(dimension, dimension, mxREAL);

	//copy matrix into temporary matrix row by row
	for (i = 0 i < dimension, i++) {
		memcpy(mxGetPr(connectivity) + dimension * i, web[i], dimension * sizeof(double));
	}
	//checker taken from lab 4
		if (engPutVariable(ep, "connectivity_matrix", connectivity_matrix)) {
			fprintf(stderr, "\nCannot write connectivity matrix to MATLAB \n");
			system("pause");
			return 1;
		}

	//make into row order instead of column order
	engEvalString(ep, "connectivity_matrix = transpose(connectivity_matrix");

	//getting the dimensions into matlab
	dimensions = mxCreateDoubleScalar(dimension) //dimensions pointer
	//checker taken from lab 4
		if (engPutVariable(ep, "dimensions", dimensions)) {
			fprintf(stderr, "\nCannot write dimensions to MATLAB \n");
			system("pause");
			return 1;
		}

	//probability factor
	if (engEvalString(ep, "p = 0.85")) {
		fprintf(stderr, "\nCannot cannot set probability factor \n");
		system("pause");
		return 1;
	}

	/*Setting up to get Stochastic Matrix*/
		//find sum of each column
		if (engPutVariable(ep, "columnsum = sum(connectivity,1)")) {
			fprintf(stderr, "\nCannot cannot calculate sum of columns \n");
			system("pause");
			return 1;
		}
		//finds the indices of the columns whose sums are != 0
		if (engEvalString(ep, "zerocolumns = find(columnsums~=0)")) {
			fprintf(stderr, "\nCannot find columnsums  zero sum indices  \n");
			system("pause");
			return 1;
		}
		//gets inverse matrix D
		if (engEvalString(ep, "D = sparse( zerocolumns, zerocolumns, 1./columnsums(zerocolumns), dimension, dimension )")) {
			fprintf(stderr, "\nCannot calculate sparse (inverse) matrix  \n");
			system("pause");
			return 1;
		}

	/*getting the stochastic matrix (transition matrix)*/
		if (engEvalString(ep, "StochasticMatrix = webMatrix * D")) {
			fprintf(stderr, "\nCannot calculate Stochastic matrix  \n");
			system("pause");
			return 1;
		}
		//finding the zero columns in connectivity_matrix
		if (engEvalString(ep, "[row, column] = find(columnsums==0)")) {
			fprintf(stderr, "\nCannot find Zero  columns  \n");
			system("pause");
			return 1;
		}
		//generate the Stochastic matrix
		if (engEvalString(ep, "StochasticMatrix(:, column) = 1./dimension")) {
			fprintf(stderr, "\nCannot generate Stochastic matrix \n");
			system("pause");
			return 1;
		}
		//generate matrix entries of the inverse of the D matrix
		if (engEvalString(ep, "Q = ones(dimension, dimension)")) {
			fprintf(stderr, "\nCannot generate Q \n");
			system("pause");
			return 1;
		}
		//generate Transition matrix
		if (engEvalString(ep, "TransitionMatrix = p * StochasticMatrix + (1 - p) * (Q/dimension)")) {
			fprintf(stderr, "\nCannot calculate Transition matrix \n");
			system("pause");
			return 1;
		}
	/*generate pagerank*/
		if (engEvalString(ep, "PageRank = ones(dimension, 1)")) {
			fprintf(stderr, "\nCannot get Page rank \n");
			system("pause");
			return 1;
		}
		//multiply column vector by Transition matrix
		if (engEvalString(ep, "for i = 1:100 PageRank = TransitionMatrix * PageRank; end")) {
			fprintf(stderr, "\nCannot calculate for Page rank \n");
			system("pause");
			return 1;
		}
		//normalize page rank vector values
		if (engEvalString(ep, "PageRank = PageRank / sum(PageRank)")) {
			fprintf(stderr, "\nCannot normalize Page rank vector \n");
			system("pause");
			return 1;
		}

//Step 3: Printing ranked list
		printf("NODE RANK\n");
		printf("---  ----\n");

		//retrieving pagerank and checking
		if ((pagerank = engGetVariable(ep, "PageRank")) == NULL) {
			fprintf(stderr, "\nFailed to retrieve PageRank\n");
			system("PAUSE");
			return 1;
		}

		else {
			for (int i = 0; i < dimension; i++) {
				printf("%d		%.4f/n", i + 1, *(mxGetPr(pagerank) + i));
			}
			printf("\n");
		}

	// To check variable list in MATLAB uncomment the block below
	// Output buffer
	if (engOutputBuffer(ep, buffer, BUFSIZE)) {
		fprintf(stderr, "\nCan't create buffer for MATLAB output\n");
		system("PAUSE");
		return 1;
	}
	buffer[BUFSIZE] = '\0';

	// Variable list
	engEvalString(ep, "whos");
	printf("%s\n", buffer);

	//clearing matrices
	mxDestroyArray(pagerank);
	mxDestroyArray(connectivity_matrix);
	mxDestroyArray(dimensions);

	//pointers are now NULL
	connectivity = NULL;
	pagerank = NULL;
	dimensions = NULL;

	//close matlab engine from lab4
	if (engClose(ep)) {
		fprintf(stderr, "\nFailed to close Matlab engine\n");
	}

//Step 4: Free dynamic memory (row by row)
	for (i = 0, i < dimension, i++) {
		free(file_pointer[i]);
		}

system("PAUSE");
return 0;
	
}




int get_maze_dimension(FILE* file_pointer) {

	int  dimension = 0;
	char line_buffer[BUFFERLINESIZE];

	dimension = strlen(fgets(line_buffer, BUFFERLINESIZE, file_pointer));

	/* You don't need to know this. It 'resets' the file's internal pointer to the
	   beginning of the file. */
	fseek(file_pointer, 0, SEEK_SET);

	/* Checks if text file was created in Windows and contains '\r'
	   IF TRUE reduce strlen by 2 in order to omit '\r' and '\n' from each line
	   ELSE    reduce strlen by 1 in order to omit '\n' from each line */
	if (strchr(line_buffer, '\r') != NULL) {
		dimension -= 2;
		// return ...
		return (dimension/2 +1);		//
	}
	else {
		dimension--;
		// return ...
		return (dimension/2+1);			//
	}
}

//code from lab 4
int** parse_maze(FILE* file_pointer, int dimension)
{
	/* Variables */
	char         line_buffer[BUFFERLINESIZE];
	int          row = 0;
	int 	   column = 0;
	int ** connectivity_matrix = NULL;

	/* Allocates memory for correctly-sized maze */
	connectivity_matrix = (int**)calloc(dimension, sizeof(int*));


	for (row = 0; row < dimension; ++row) {
		connectivity_matrix[row] = (int*)calloc(dimension, sizeof(int));
	}

	row = 0;
	while (fgets(line_buffer, BUFFERLINESIZE, file_pointer)) {
		for (column = 0; column < dimension; ++column) {
			//allows for spaces in txt file
			connectivity_matrix[row][column] = line_buffer[column*2]-'0';
		}
		row++;
	}
	return connectivity_matrix;
}