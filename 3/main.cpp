#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

double myRand(double min, double max) {
	return (max - min) * ((double)rand() / (double)RAND_MAX) + min;
}

void initSigmas(double* sigma, int** letters, int h, int n) { // выбираем сигма как половину расстояния до ближайшего центра ячейки (в 36-мерном пр-ве)
	// для каждого класса (идеального образа) находим расстояние до ближайшего. половина этого расстояния - сигма
	for (int c = 0; c < h; c++) {
		double min = -1;
		for (int k = 0; k < h; k++) {
			if (k == c) {
				continue;
			}

			double sum = 0;
			for (int i = 0; i < n; i++) {
				sum += ((double)letters[c][i] - (double)letters[k][i])*((double)letters[c][i] - (double)letters[k][i]);
			}
			sum = sqrt(sum) / 2;
			if (sum < min || min < 0) {
				min = sum;
			}
		}
		sigma[c] = min;
	}
}

void learn(int** letters, double* g, double* y, double** w, double* sigma, int p, int n, int h, int m) {
	for (int j = 0; j < h; j++) {
		for (int k = 0; k < m; k++) {
			w[j][k] = myRand(-1, 1);
		} // рандомно инициализируем веса
	}

	int* x;
	double alpha = 0.5; // эмпирически подобрано
	double* d = new double[m];
	double D = 0.000001;
	unsigned long long int iteration = 1;
	double dmax;

	do {
		dmax = 0;
		for (int c = 0; c < p; c++) {
			x = letters[c];

			for (int j = 0; j < h; j++) {
				double sum = 0;
				for (int i = 0; i < n; i++) {
					sum += (x[i] - letters[j][i])*(x[i] - letters[j][i]);
				}
				g[j] = exp(-sum / (sigma[j] * sigma[j])); // колокол Гаусса
			}

			for (int k = 0; k < m; k++) {
				double sum = 0;
				for (int j = 0; j < h; j++) {
					sum += w[j][k] * g[j];
				}
				y[k] = sum;
			}

			for (int k = 0; k < m; k++) {
				double yr = 0;
				if (k == c) {
					yr = 1;
				}
				d[k] = yr - y[k];
				if (dmax < abs(d[k])) {
					dmax = abs(d[k]);
				}
			}

			for (int k = 0; k < m; k++) {
				for (int j = 0; j < h; j++) {
					w[j][k] += alpha * d[k] * g[j]; // корректируем веса
				}
			}
		}
		iteration++;
		cout << iteration << " " << dmax << endl;

	} while (dmax > D);

	delete[] d;
}

void result(int* x, double* g, double* y, double** w, double* sigma, int** letters, int n, int p, int h, int m) {
	for (int j = 0; j < h; j++) {
		double sum = 0;
		for (int i = 0; i < n; i++) {
			sum += (x[i] - letters[j][i])*(x[i] - letters[j][i]);
		}
		g[j] = exp(-sqrt(sum) / (sigma[j] * sigma[j]));
	}

	for (int k = 0; k < m; k++) {
		double sum = 0;
		for (int j = 0; j < h; j++) {
			sum += w[j][k] * g[j];
		}
		y[k] = sum;
	}
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
	int p, m, h;
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

	m = p;
	h = m; // h - количество нейронов промежуточного слоя (равно количеству классов), а у нас каждый идеальный образ - отдельный класс

	double* g = new double[h];
	double* y = new double[m];
	double* sigma = new double[h];

	initSigmas(sigma, letters, h, n*n);

	double** w = new double*[h];
	for (int i = 0; i < h; i++) {
		w[i] = new double[m];
		for (int j = 0; j < m; j++) {
			w[i][j] = 0;
		}
	}

	learn(letters, g, y, w, sigma, p, n*n, h, m);

	int* testLetter = new int[n*n];

	ofstream out;
	out.open("output.txt");

	for (int c = 0; c < p; c++) {
		out << "image number: " << c << endl;
		for (int percent = 10; percent <= 30; percent += 10) {
			out << "noise percent: " << percent << endl;
			addError(n * n, letters[c], testLetter, percent); // зашумливаем буковки
			out << "test image: " << endl;

			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					out << testLetter[n * i + j] << " ";
				}
				out << endl;
			}
			out << endl;

			result(testLetter, g, y, w, sigma, letters, n*n, p, h, m);

			out << "result: ";
			for (int i = 0; i < m; i++) {
				out << (y[i] * 100) << " ";
			}
			out << endl << endl;
		}
	}

	out.close();

	delete[] testLetter;

	for (int i = 0; i < h; i++) {
		delete[] w[i];
	}
	delete[] w;

	for (int c = 0; c < p; c++) {
		delete[] letters[c];
	}
	delete[] letters;
	delete[] g;
	delete[] y;
	delete[] sigma;

	return 0;
}