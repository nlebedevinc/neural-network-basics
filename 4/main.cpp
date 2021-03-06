#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>
using namespace std;

double myRand(double min, double max) {
	return (max - min) * ((double)rand() / (double)RAND_MAX) + min;
}

void learn(int** letters, double* y, double** w, int p, int n, int m) {
	for (int j = 0; j < m; j++) {
		double length = 0;
		for (int i = 0; i < n; i++) {
			w[i][j] = myRand(-1, 1);
			length += w[i][j] * w[i][j];
		}
		length = sqrt(length);
		for (int i = 0; i < n; i++) {
			w[i][j] /= length; // нормировка
		}
	}

	double* x = new double[n];
	double betha = 0.8;
	double* d = new double[m];
	int* f = new int[m]; // частоты побед синапсов
	for (int i = 0; i < m; i++) {
		f[i] = 1;
	}
	double D = 0.1;
	unsigned long long int iteration = 1;
	unsigned long long int lastIteration = 1;
	double dmax, lastDmax = FLT_MAX;

	do {
		dmax = 0;
		for (int c = 0; c < p; c++) {
			double length = 0;
			double dv = FLT_MAX;
			int dvindex;
			for (int i = 0; i < n; i++) {
				length += letters[c][i] * letters[c][i];
			}
			length = sqrt(length);
			for (int i = 0; i < n; i++) {
				x[i] = (double)letters[c][i] / length;
			} // нормируем x

			for (int j = 0; j < m; j++) {
				double sum = 0;
				for (int i = 0; i < n; i++) {
					sum += w[i][j] * x[i];
				}
				y[j] = sum;
			}

			for (int j = 0; j < m; j++) {
				double sum = 0;
				for (int i = 0; i < n; i++) {
					sum += (x[i] - w[i][j]) * (x[i] - w[i][j]);
				}
				sum = sqrt(sum) * f[j]; // ошибка (расстояние * частоту побед)
				if (dv > sum) {
					dv = sum;
					dvindex = j;
				}
			}

			length = 0;
			for (int i = 0; i < n; i++) {
				w[i][dvindex] += betha * (x[i] - w[i][dvindex]); // сдвигаем столбец-победитель
				length += w[i][dvindex] * w[i][dvindex];
			}
			length = sqrt(length);
			for (int i = 0; i < n; i++) {
				w[i][dvindex] /= length; // нормируем
			}
			f[dvindex]++; // увеличиваем победы

			double dist = 0;
			for (int i = 0; i < n; i++) {
				dist += (x[i] - w[i][dvindex]) * (x[i] - w[i][dvindex]);
			}
			dist = sqrt(dist);

			if (dmax < dist) {
				dmax = dist; // находим максимальное расстояние
			}
		}
		iteration++;
		if (dmax < lastDmax) {
			lastDmax = dmax;
			lastIteration = iteration;
		}
		cout << iteration << " " << dmax << endl;

		if (dmax == lastDmax && iteration - lastIteration > 100000) { // оцениваем кол-ва итераций (специфика того, что синапсов меньше, чем идеальных образов)
			break;
		}
	} while (dmax > D);

	delete[] d;
	delete[] x;
	delete[] f;
}

void result(int* x, double* y, double** w, int n, int p, int m) {
	double* v = new double[n];

	double sum = 0;
	for (int i = 0; i < n; i++) {
		sum += x[i] * x[i];
	}
	sum = sqrt(sum);
	for (int i = 0; i < n; i++) {
		v[i] = (double)x[i] / sum;
	}

	for (int j = 0; j < m; j++) {
		double sum = 0;
		for (int i = 0; i < n; i++) {
			sum += w[i][j] * v[i];
		}
		y[j] = sum;
	}

	delete[] v;
}

void addError(int n, int* letter, int* withError, int percent) {
	for (int i = 0; i < n; i++) {
		withError[i] = letter[i];
	}

	bool* flags = new bool[n];
	for (int i = 0; i < n; i++) {
		flags[i] = false;
	}

	int count = (n * percent) / 100;

	while (count) {
		int index = rand() % n;
		if (!flags[index]) {
			flags[index] = true;
			withError[index] = 1 - withError[index];
			count--;
		}
	}

	delete[] flags;
}

int main() {
	int p, m;
	int n = 6;
	int ** letters;

	srand(time(NULL));
	ifstream in;

	in.open("input.txt");

	if (!in) {
		cerr << "Unable to open file input.txt";
		return (1);   // call system to stop
	}
	in >> p;

	letters = new int*[p];
	for (int c = 0; c < p; c++) {
		letters[c] = new int[n*n];
		for (int i = 0; i < n*n; i++) {
			in >> letters[c][i];
			cout << letters[c][i] << " ";
		}
		cout << endl << endl;
	}
	in.close();

	m = p - 1; // классов на один меньше, чем идеальных образов

	double* y = new double[m];

	double** w = new double*[n*n];
	for (int i = 0; i < n*n; i++) {
		w[i] = new double[m];
		for (int j = 0; j < m; j++) {
			w[i][j] = 0;
		}
	}

	learn(letters, y, w, p, n*n, m);

	int* testLetter = new int[n*n];

	ofstream out;
	out.open("output.txt");

	for (int c = 0; c < p; c++) {
		out << "image number: " << c << endl;
		for (int percent = 0; percent <= 30; percent += 10) {
			out << "noise percent: " << percent << endl;
			addError(n * n, letters[c], testLetter, percent);
			out << "test image: " << endl;

			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					out << testLetter[n * i + j] << " ";
				}
				out << endl;
			}
			out << endl;

			result(testLetter, y, w, n*n, p, m);

			out << "result: ";
			for (int i = 0; i < m; i++) {
				out << (y[i] * 100) << " ";
			}
			out << endl << endl;
		}
	}

	out.close();

	delete[] testLetter;

	for (int i = 0; i < n; i++) {
		delete[] w[i];
	}
	delete[] w;

	for (int c = 0; c < p; c++) {
		delete[] letters[c];
	}
	delete[] letters;
	delete[] y;

	return 0;
}