#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <fstream>
#include <iostream>

using namespace std;

void Initialization(string& ppmFormat,
	int& width, int& height, int& maxColor, unsigned char*& image, int& RowNum, unsigned char*& res) {

	ifstream input;
	input.open("C:\\Users\\38067\\source\\repos\\PictureBinarization\\inputPicture.ppm", ios::binary);
	if (!input.is_open()) {
		cout << "Can't open input file";
		exit(1);
	}
	input >> ppmFormat;
	input >> width >> height;
	input >> maxColor;

	image = new unsigned char[height * width];

	char r;
	input.get(r);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			char r, g, b;
			input.get(r);
			input.get(g);
			input.get(b);
			image[i * width + j] = 0.2125 * double(r) + 0.7154 * double(g) + 0.0721 * double(b);
		}
	}
	input.close();

	res = new unsigned char[height * width];
}

void Termination(string& ppmFormat,
	int& width, int& height, int& maxColor, unsigned char*& image, unsigned char*& res) {

	fstream output;

	output.open("C:\\Users\\38067\\source\\repos\\PictureBinarization\\outputPicture.ppm", ios::out | ios::binary);
	output << ppmFormat << '\n';
	output << width << ' ' << height << '\n';
	output << maxColor << '\n';
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			output << (char)res[i * width + j] << (char)res[i * width + j] << (char)res[i * width + j];
		}
	}

	output.close();
	delete[] image;
	delete[] res;
}

void BradleyThreshold(unsigned char* src, unsigned char* res, int width, int height) {
	const int S = width / 8;
	int s2 = S / 2;
	const float t = 0.30;
	unsigned long* integral_image = 0;
	long sum = 0;
	int count = 0;
	int index;
	int x1, y1, x2, y2;

	integral_image = new unsigned long[width * height * sizeof(unsigned long*)];

	for (int i = 0; i < width; i++) {
		sum = 0;
		for (int j = 0; j < height; j++) {
			index = j * width + i;
			sum += src[index];
			if (i == 0)
				integral_image[index] = sum;
			else
				integral_image[index] = integral_image[index - 1] + sum;
		}
	}

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			index = j * width + i;

			x1 = i - s2;
			x2 = i + s2;
			y1 = j - s2;
			y2 = j + s2;

			if (x1 < 0)
				x1 = 0;
			if (x2 >= width)
				x2 = width - 1;
			if (y1 < 0)
				y1 = 0;
			if (y2 >= height)
				y2 = height - 1;

			count = (x2 - x1) * (y2 - y1);

			sum = integral_image[y2 * width + x2] - integral_image[y1 * width + x2] -
				integral_image[y2 * width + x1] + integral_image[y1 * width + x1];
			if ((long)(src[index] * count) < (long)(sum * (1.0 - t)))
				res[index] = 0;
			else
				res[index] = 255;
		}
	}

	delete[] integral_image;
}

void Serial() {
	string ppmFormat;
	int width, height;
	int maxColor;
	unsigned char* image;
	unsigned char* procImage;
	unsigned char* procRes;
	unsigned char* res;
	double Start, Finish;

	ifstream input;
	input.open("C:\\Users\\38067\\source\\repos\\PictureBinarization\\inputPicture.ppm", ios::binary);
	if (!input.is_open()) {
		cout << "Can't open input file";
		exit(1);
	}
	input >> ppmFormat;
	input >> width >> height;
	input >> maxColor;

	image = new unsigned char[height * width];

	char r;
	input.get(r);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			char r, g, b;
			input.get(r);
			input.get(g);
			input.get(b);
			image[i * width + j] = 0.2125 * double(r) + 0.7154 * double(g) + 0.0721 * double(b);
		}
	}
	input.close();

	res = new unsigned char[height * width];

	Start = clock();
	BradleyThreshold(image, res, width, height);
	Finish = clock();
	printf("Time of execution = %f\n", (Finish - Start) / CLK_TCK);

	fstream output;

	output.open("C:\\Users\\38067\\source\\repos\\PictureBinarization\\outputPicture.ppm", ios::out | ios::binary);
	output << ppmFormat << '\n';
	output << width << ' ' << height << '\n';
	output << maxColor << '\n';
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			output << (char)res[i * width + j] << (char)res[i * width + j] << (char)res[i * width + j];
		}
	}

	output.close();
	delete[] image;
	delete[] res;
}

int main(int argc, char* argv[]) {
	Serial();
	return 0;
	string ppmFormat;
	int width, height;
	int maxColor, rowNum;
	unsigned char* image;
	unsigned char* res;
	double Start, Finish;

	Initialization(ppmFormat, width, height, maxColor, image, rowNum, res);
	int threads = omp_get_num_procs();

	Start = clock();
#pragma omp parallel for num_threads(threads)// Fork threads
	for(int thread = 0; thread < threads; thread++){
		int startRow = (height / threads)*thread;
		startRow += min(thread, height % threads);
		BradleyThreshold(image + startRow, res + startRow, width, (height / threads) + (thread < (height% threads)));
	} // Join threads

	Finish = clock();
	Termination(ppmFormat, width, height, maxColor, image, res);

	printf("Time of execution = %f\n", (Finish - Start) / CLK_TCK);
}