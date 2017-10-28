// LabPPD3NrComplexe.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <iostream>
#include <complex>
#include <fstream>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <thread>

int no_threads, n1, m1, n2, m2;

class complex {

protected:
	double r;
	double i;

public:
	complex() {};
	complex(double r, double i) {
		this->r = r;
		this->i = i;
	}
	complex(const complex& c);

	complex operator+ (const complex& c) {
		return complex(c.r + this->r, c.i + this->i);
	}
	friend std::ostream &operator<< (std::ostream &output, complex &c);
	friend std::istream &operator>> (std::istream &input, complex& c);

};

std::istream& operator>> (std::istream &input, complex& c) {
	char plus;
	char i;
	input >> c.r >> plus >> c.i >> i;
	return input;
}
std::ostream &operator<< (std::ostream &output, complex &c) {
	output << c.r << "+" << c.i << "i";
	return output;
}

complex matrix1[5000][5000];
complex matrix2[5000][5000];
complex matrix3[5000][5000];


void generateMatrix(int threads, int n, int m, int n2, int m2)
{
	srand(time(NULL));
	std::ofstream f;
	f.open("matrix-data.txt");

	f << threads << " " << n << " " << m << " " << n2 << " " << m2 << std::endl;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			f << (rand() % 10) << "+";
			f << (rand() % 10) << "i ";
		}
		f << std::endl;
	}

	f << std::endl;

	for (int i = 0; i < n2; i++) {
		for (int j = 0; j < m2; j++) {
			f << (rand() % 10) << "+";
			f << (rand() % 10) << "i ";
		}
		f << std::endl;
	}
	f.close();
}

void readData() {
	std::ifstream f("matrix-data.txt");
	f >> no_threads;
	f >> n1;
	f >> m1;
	f >> n2;
	f >> m2;


	for (int i = 0; i < n1; i++)
		for (int j = 0; j < m1; j++) {
			f >> matrix1[i][j];
		}
	for (int i = 0; i < n2; i++)
		for (int j = 0; j < m2; j++) {
			f >> matrix2[i][j];
		}
	f.close();
}


void addComplexOptimised(complex matrix1[5000][5000], complex matrix2[5000][5000], complex matrix3[5000][5000], int iStart, int iStop, int jStart, int jStop, int n) {
	int iStartLocal = iStart;
	int iStopLocal = iStop;
	if (jStart > 0) {
		iStartLocal++;
		for (int i = jStart; i < n; i++) {
			matrix3[iStart][i] = matrix1[iStart][i] + matrix2[iStart][i];
		}
	}
	if (jStop < n) {
		iStopLocal--;
		for (int i = 0; i < jStop; i++) {
			matrix3[iStop][i] = matrix1[iStart][i] + matrix2[iStart][i];
		}
	}
	for (int i = iStart; i <= iStopLocal; i++) {
		for (int j = 0; j < n; j++) {
			matrix3[i][j] = matrix1[i][j] + matrix2[i][j];
		}
	}
}


void parallelComplexAddOptimised(complex matrix1[5000][5000], complex matrix2[5000][5000], complex matrix3[5000][5000], int nrThreaduri, int n) {
	int operatiiThread = (n * n) / nrThreaduri;
	int rest = (n * n) % nrThreaduri;

	int iStart = 0, jStart = 0, iStop = 0, jStop = 0;

	std::vector<std::thread> threads;

	for (int i = 0; i < nrThreaduri; ++i) {
		int operatiiFinal;
		if (rest > 0) {
			operatiiFinal = operatiiThread + 1;
			rest--;
		}
		else {
			operatiiFinal = operatiiThread;
		}
		while (operatiiFinal != 0) {
			if (jStop == n) {
				jStop = 0;
				iStop++;
			}
			jStop++;
			if (jStop == n) {
				jStop = 0;
				iStop++;
			}
			operatiiFinal--;
		}

		//////////////////////////////
		std::thread t(addComplexOptimised, matrix1, matrix2, matrix3, iStart, iStop, jStart, jStop, n);
		threads.push_back(move(t));

		iStart = iStop;
		jStart = jStop + 1;
		if (jStart > n) {
			jStart = 0;
			++iStart;
		}

	}
	long start = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	for (std::thread &t : threads) {
		t.join();
	}
	long end = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	std::cout << "parallel add execution time: " << (end - start) << " ms" << std::endl;
}

void printResult(complex matrix3[5000][5000], int n) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			std::cout << matrix3[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

int main() {
	generateMatrix(5, 10, 10, 10, 10);
	readData();
	parallelComplexAddOptimised(matrix1, matrix2, matrix3, no_threads, n1);
	printResult(matrix3, n1);
	system("PAUSE");
	return 0;
}