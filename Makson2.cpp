#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

struct Flixel {
    int number; // номер в текстовом файле
    int R;      // красный канал
    int G;      // зелёный канал
    int B;      // голубой канал
};

// Чтение данных из текстового файла в массив
vector<int> fileTxtToIntArray(const string& filePath) {
    ifstream file(filePath);
    vector<int> array;
    if (!file.is_open()) {
        cerr << "Ошибка открытия файла: " << filePath << endl;
        return array;
    }
    int value;
    while (file >> value) {
        array.push_back(value);
    }
    return array;
}

// Нахождение максимального значения в массиве
int getMax(const vector<int>& array) {
    return *max_element(array.begin(), array.end());
}

// Сортировка подсчётом для radix sort
void countSort(vector<Flixel>& array, int exp) {
    int size = array.size();
    vector<Flixel> output(size);
    vector<int> count(10, 0);

    for (int i = 0; i < size; i++)
        count[(array[i].number / exp) % 10]++;

    for (int i = 1; i < 10; i++)
        count[i] += count[i - 1];

    for (int i = size - 1; i >= 0; i--) {
        output[count[(array[i].number / exp) % 10] - 1] = array[i];
        count[(array[i].number / exp) % 10]--;
    }

    for (int i = 0; i < size; i++)
        array[i] = output[i];
}

// Radix sort
void radixSort(vector<int>& numbers, vector<Flixel>& pixels) {
    int maxVal = getMax(numbers);
    for (int exp = 1; maxVal / exp > 0; exp *= 10) {
        countSort(pixels, exp);
    }
}

int main() {
    string txtPath = "C:/Users/LISA/Desktop/prog/WinFormsApp1/28.txt";
    string imgPath = "C:/Users/Павел/Хорошин/Desktop/WinFormsApp1/28e.bmp";
    string savePath = "C:/Users/Павел/Хорошин/Desktop/WinFormsApp1/savefile.bmp";

    // Чтение чисел из текстового файла
    vector<int> elements = fileTxtToIntArray(txtPath);
    if (elements.empty()) {
        cerr << "Ошибка чтения текстового файла" << endl;
        return -1;
    }

    // Загрузка изображения
    Mat image = imread(imgPath, IMREAD_COLOR);
    if (image.empty()) {
        cerr << "Ошибка загрузки изображения" << endl;
        return -1;
    }

    int sizeImage = image.rows * image.cols;
    if (elements.size() != sizeImage) {
        cerr << "Несоответствие размеров данных и изображения" << endl;
        return -1;
    }

    vector<Flixel> allFlixels(sizeImage);
    for (int i = 0; i < sizeImage; i++) {
        allFlixels[i].number = elements[i];
    }

    // Заполнение массива цветов из изображения
    int counter = 0;
    for (int i = image.rows - 1; i >= 0; i--) {
        for (int j = 0; j < image.cols; j++) {
            Vec3b color = image.at<Vec3b>(i, j);
            allFlixels[counter].R = color[2]; // R
            allFlixels[counter].G = color[1]; // G
            allFlixels[counter].B = color[0]; // B
            counter++;
        }
    }

    // Сортировка
    radixSort(elements, allFlixels);

    // Применение сортированных цветов к изображению
    counter = 0;
    for (int i = image.rows - 1; i >= 0; i--) {
        for (int j = 0; j < image.cols; j++) {
            image.at<Vec3b>(i, j) = Vec3b(allFlixels[counter].B, allFlixels[counter].G, allFlixels[counter].R);
            counter++;
        }
    }

    // Сохранение обработанного изображения
    imwrite(savePath, image);
    cout << "Обработанное изображение сохранено в: " << savePath << endl;

    return 0;
}