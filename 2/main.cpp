#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
using namespace std;

double f(double x) {
	return 1.0 / (1 + exp(-x));
}

double myRand(double min, double max) {
	return (max - min) * ((double)rand() / (double)RAND_MAX) + min;
}

void learn(int** letters, double* g, double* y, double** v, double* Q, double** w, double* T, int p, int n, int h, int m) {
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < h; j++) {
			v[i][j] = myRand(-1, 1);
		}
	}
	for (int j = 0; j < h; j++) {
		Q[j] = myRand(-1, 1);
	}
	for (int j = 0; j < h; j++) {
		for (int k = 0; k < m; k++) {
			w[j][k] = myRand(-1, 1);
		}
	}
	for (int k = 0; k < m; k++) {
		T[k] = myRand(-1, 1);
	} // инициализируем v, w, Q, T рандомом от -1 до 1

	int* x;
	double alpha = 3, betha = 2; // эмпирическим путем подобраны
	double* d = new double[m];
	double* e = new double[h];
	double D = 0.0001; // минимальная степень 10-ки, за которую сеть учится адекватное время (до 5 минут)
	unsigned long long int iteration = 1; // счетчик количества итераций
	double dmax, outD = 1; // dmax - максимальная ошибка, outD - промежуточные ошибки (визуализация того, что обучение идет)

	do {
		dmax = 0;
		for (int c = 0; c < p; c++) {
			x = letters[c];

			for (int j = 0; j < h; j++) {
				double sum = Q[j];
				for (int i = 0; i < n; i++) {
					sum += v[i][j] * x[i];
				}
				g[j] = f(sum);
			}

			for (int k = 0; k < m; k++) {
				double sum = T[k];
				for (int j = 0; j < h; j++) {
					sum += w[j][k] * g[j];
				}
				y[k] = f(sum);
			}

			for (int k = 0; k < m; k++) {
				double yr = 0;
				if (k == c) {
					yr = 1; // строим эталонный выход
				}
				d[k] = yr - y[k]; // считаем ошибочку
				if (dmax < abs(d[k])) {
					dmax = abs(d[k]);
				}
			}

			for (int j = 0; j < h; j++) {
				e[j] = 0;
				for (int k = 0; k < m; k++) {
					e[j] += d[k] * y[k] * (1 - y[k]) * w[j][k]; // вычисляем ошибки скрытого слоя
				}
			}

			for (int k = 0; k < m; k++) {
				for (int j = 0; j < h; j++) {
					w[j][k] += alpha * y[k] * (1 - y[k]) * d[k] * g[j];
				}

				T[k] += alpha * y[k] * (1 - y[k]) * d[k];
			}

			for (int j = 0; j < h; j++) {
				for (int i = 0; i < n; i++) {
					v[i][j] += betha * g[j] * (1 - g[j]) * e[j] * x[i];
				}
				Q[j] += betha * g[j] * (1 - g[j]) * e[j]; // по формулам корректируем веса и пороги
			}
		}
		iteration++;
		if (dmax < outD - 0.00001) {
			cout << dmax << endl;
			outD = dmax;
		}
	} while (dmax > D);

	cout << "Iteration: " << iteration;
	delete[] d;
	delete[] e;
}

void result(int* x, double* g, double* y, double** v, double* Q, double** w, double* T, int n, int h, int m) {
	for (int j = 0; j < h; j++) {
		double sum = Q[j];
		for (int i = 0; i < n; i++) {
			sum += v[i][j] * x[i];
		}
		g[j] = f(sum); // формулы воспроизвадения
	}

	for (int k = 0; k < m; k++) {
		double sum = T[k];
		for (int j = 0; j < h; j++) {
			sum += w[j][k] * g[j];
		}
		y[k] = f(sum);
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

	m = p; // единичка на конкретном синапсе соответствует конкретному идеальному образу (образов p, значит и выходов p)
	h = 5 * ceil((double)p / (n * n)); // h пропорционально p/n^2

	double* g = new double[h];
	double* y = new double[m];
	double* Q = new double[h];
	double* T = new double[m];

	double** w = new double*[h];
	for (int i = 0; i < h; i++) {
		w[i] = new double[m];
		for (int j = 0; j < m; j++) {
			w[i][j] = 0;
		}
	}

	double** v = new double*[n*n];
	for (int i = 0; i < n*n; i++) {
		v[i] = new double[h];
		for (int j = 0; j < h; j++) {
			v[i][j] = 0;
		}
	}

	learn(letters, g, y, v, Q, w, T, p, n*n, h, m);

	int* testLetter = new int[n*n]; // для образов с шумами

	ofstream out;
	out.open("output.txt");

	for (int c = 0; c < p; c++) {
		out << "image number: " << c << endl;
		for (int percent = 10; percent <= 30; percent += 10) {
			out << "noise percent: " << percent << endl;
			addError(n * n, letters[c], testLetter, percent); // зашумливаем
			out << "test image: " << endl;

			for (int i = 0; i < n; i++) {
				for (int j = 0; j < n; j++) {
					out << testLetter[n * i + j] << " ";
				}
				out << endl;
			}
			out << endl;

			result(testLetter, g, y, v, Q, w, T, n*n, h, m); // смотрим результат

			out << "result: ";
			for (int i = 0; i < m; i++) {
				out << (y[i] * 100) << " "; // выводим процент похожести
			}
			out << endl << endl;
		}
	}

	out.close();

	delete[] testLetter;
	for (int i = 0; i < n*n; i++) {
		delete[] v[i];
	}
	delete[] v;
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
	delete[] Q;
	delete[] T;

	return 0;
}