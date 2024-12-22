#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include "pixel.h"

using namespace std;

void swap_pixels(BMP& bmp, int i, int j) {
    for (int k = 0; k < 3; ++k) {
        uint8_t temp = bmp.data[i * 3 + k];
        bmp.data[i * 3 + k] = bmp.data[j * 3 + k];
        bmp.data[j * 3 + k] = temp;
    }
}

int partition(vector<int>& arr, BMP& bmp, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; ++j) {
        if (arr[j] < pivot) {
            ++i;
            swap(arr[i], arr[j]);
            swap_pixels(bmp, i, j);
        }
    }

    swap(arr[i + 1], arr[high]);
    swap_pixels(bmp, i + 1, high);

    return i + 1;
}

void quicksort(vector<int>& arr, BMP& bmp, int low, int high, int depth_limit) {
    while (low < high) {
        if (depth_limit == 0) {
            make_heap(arr.begin() + low, arr.begin() + high + 1);
            sort_heap(arr.begin() + low, arr.begin() + high + 1);
            return;
        }

        int pi = partition(arr, bmp, low, high);

        if (pi - low < high - pi) {
            quicksort(arr, bmp, low, pi - 1, depth_limit);
            low = pi + 1;
        } else {
            quicksort(arr, bmp, pi + 1, high, depth_limit);
            high = pi - 1;
        }
    }
}

void insertion_sort(vector<int>& arr, BMP& bmp, int low, int high) {
    for (int i = low + 1; i <= high; ++i) {
        int key = arr[i];
        int j = i - 1;

        while (j >= low && arr[j] > key) {
            arr[j + 1] = arr[j];
            swap_pixels(bmp, j + 1, j);
            --j;
        }
        arr[j + 1] = key;
    }
}

void introsort(vector<int>& arr, BMP& bmp, int low, int high) {
    int size = high - low + 1;

    if (size <= 16) {
        insertion_sort(arr, bmp, low, high);
    } else {
        int depth_limit = 2 * log(size);
        quicksort(arr, bmp, low, high, depth_limit);
    }
}

void fl(BMP& bmp) {
    int width = bmp.bmp_info_header.width;
    int height = bmp.bmp_info_header.height;

    for (int i = 0; i < height / 2; ++i) {
        for (int j = 0; j < width; ++j) {
            int idx1 = (i * width + j) * 3;
            int idx2 = ((height - 1 - i) * width + j) * 3;

            for (int k = 0; k < 3; ++k) {
                uint8_t temp = bmp.data[idx1 + k];
                bmp.data[idx1 + k] = bmp.data[idx2 + k];
                bmp.data[idx2 + k] = temp;
            }
        }
    }
}

// int main() {
//     vector<int> Chisla;
//     BMP bmp("/Users/nizamialekperov/Desktop/untitled3 2/1.bmp"); //наш bmp файл
//
//     ifstream fin("/Users/nizamialekperov/Desktop/untitled3 2/1.txt"); //наш txt файл
//     int Chislo;
//
//     while (fin >> Chislo) {
//         Chisla.push_back(Chislo);
//     }
//     fin.close();
//
//     introsort(Chisla, bmp, 0, Chisla.size() - 1);
//     fl(bmp);
//     bmp.write("/Users/nizamialekperov/Desktop/untitled3 2/NewBmp!.bmp"); //куда выводить полученный bmp файл
//
//     return 0;
// }