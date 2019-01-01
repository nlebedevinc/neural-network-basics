#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

template<class T>
using Matrix = std::vector<std::vector<T>>;

// Структура объекта картинки
struct Image {
	std::vector<int> vector;
	char character;

	int operator[](size_t i) const {
		return vector[i];
	}

	size_t size() const {
		return vector.size();
	}
};

std::vector<Image> read_images(const std::string &filename);
Matrix<int> train(const std::vector<Image> &a);
char recognize(const std::vector<int> &v, const Matrix<int> &w, const std::vector<Image> &imgs);

template<class T>
void print(const Matrix<T> &m);

int main() {
	std::vector<Image> images = read_images("data/images.txt");
	Matrix<int> w = train(images);

	std::vector<Image> tests = read_images("data/tests.txt");

	std::cout << recognize(tests[0].vector, w, images) << std::endl;

	return 0;
}

std::vector<Image> read_images(const std::string &filename) {
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "KARAUL!!!!\n";
		exit(1);
	}
	size_t image_count, vector_size;
	file >> image_count >> vector_size;
	std::vector<Image> images;
	images.reserve(image_count);

	for (size_t i = 0; i < image_count; ++i) {
		images.emplace_back();
		file >> images[i].character;
		images[i].vector.reserve(vector_size);
		for (size_t j = 0; j < vector_size; ++j) {
			char c;
			file >> c;
			images[i].vector.push_back('*' == c ? 1 : -1);
		}
	}

	return images;
}

// Составляем матрицу весов
Matrix<int> train(const std::vector<Image> &a) {
	size_t size = a[0].size();
	Matrix<int> w(size, std::vector<int>(size));

	for (size_t i = 0; i < size; ++i)
		for (size_t j = 0; j < size; ++j)
			if (i != j)
				for (size_t k = 0; k < a.size(); ++k)
					w[i][j] += a[k][i] * a[k][j];

	return w;
}

template<class T>
void print(const Matrix<T> &m) {
	for (size_t i = 0; i < m.size(); ++i) {
		std::copy(m[i].begin(), m[i].end(), std::ostream_iterator<T>(std::cout, " "));
		std::cout << "\n";
	}
}

// перемножение исходной мастрицы на вектор
std::vector<int> mul(const Matrix<int> &m, const std::vector<int> &v) {
	std::vector<int> res(v.size());

	for (size_t i = 0; i < res.size(); ++i) {
		for (size_t j = 0; j < res.size(); ++j)
			res[i] += m[i][j] * v[j];
	}

	return res;
}

std::vector<int> f(const std::vector<int> &v) {
	std::vector<int> res{v};
	for (int &x : res)
		x = (x > 0 ? 1 : -1);
	return res;
}

char recognize(const std::vector<int> &v, const Matrix<int> &w, const std::vector<Image> &imgs) {
	std::vector<int> prev{v};

	std::vector<int> cur = f(mul(w, v));

	while (cur != prev) {
		prev = cur;
		cur = f(mul(w, cur));
	}
	
	std::cout << "Recognition result:\n";
	for (size_t i = 0; i < 10; ++i) {
		for (size_t j = 0; j < 10; ++j)
			std::cout << (cur[i * 10 + j] > 0 ? '*' : '-') << " ";
		std::cout << '\n';
	}

	for (const auto &i : imgs)
		if (cur == i.vector)
			return i.character;
	return '?';
}