#pragma once
#include <fstream>
#include <vector>
#include <stdexcept>
#include <iostream>

#pragma pack(push, 1)

struct BMPFileHeader {
    uint16_t file_type{ 0x4D42 };
    uint32_t file_size{ 0 };
    uint16_t reserved1{ 0 };
    uint16_t reserved2{ 0 };
    uint32_t offset_data{ 0 };
};

struct BMPInfoHeader {
    uint32_t size{ 0 };
    int32_t width{ 0 };
    int32_t height{ 0 };
    uint16_t planes{ 1 };
    uint16_t bit_count{ 0 };
    uint32_t compression{ 0 };
    uint32_t size_image{ 0 };
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };
    uint32_t colors_important{ 0 };
};

struct BMPColorHeader {
    uint32_t red_mask{ 0x00ff0000 };
    uint32_t green_mask{ 0x0000ff00 };
    uint32_t blue_mask{ 0x000000ff };
    uint32_t alpha_mask{ 0xff000000 };
    uint32_t color_space_type{ 0x73524742 };
    uint32_t unused[16]{ 0 };
};
#pragma pack(pop)

struct BMP {
    BMPFileHeader file_header;
    BMPInfoHeader bmp_info_header;
    BMPColorHeader bmp_color_header;
    std::vector<uint8_t> data;

    BMP(const char* fname) {
        read(fname);
    }

    void read(const char* fname) {
        std::ifstream inp{ fname, std::ios_base::binary };
        if (inp) {
            inp.read((char*)&file_header, sizeof(file_header));
            if (file_header.file_type != 0x4D42) {
                throw std::runtime_error("ошибка! неизвестный формат файла");
            }
            inp.read((char*)&bmp_info_header, sizeof(bmp_info_header));

            if (bmp_info_header.bit_count == 32) {
                if (bmp_info_header.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
                    inp.read((char*)&bmp_color_header, sizeof(bmp_color_header));
                    check_color_header(bmp_color_header);
                }
                else {
                    throw std::runtime_error("ошибка! неизвестный формат файла");
                }
            }

            inp.seekg(file_header.offset_data, inp.beg);

            if (bmp_info_header.bit_count == 32) {
                bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
                file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            }
            else {
                bmp_info_header.size = sizeof(BMPInfoHeader);
                file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
            }
            file_header.file_size = file_header.offset_data;

            if (bmp_info_header.height < 0) {
                throw std::runtime_error("программа может обрабатывать только bmp-изображения с началом координат в нижнем левом углу");
            }

            data.resize(bmp_info_header.width * bmp_info_header.height * bmp_info_header.bit_count / 8);

            if (bmp_info_header.width % 4 == 0) {
                inp.read((char*)data.data(), data.size());
                file_header.file_size += static_cast<uint32_t>(data.size());
            }
            else {
                row_stride = bmp_info_header.width * bmp_info_header.bit_count / 8;
                uint32_t new_stride = make_stride_aligned(4);
                std::vector<uint8_t> padding_row(new_stride - row_stride);

                for (int y = 0; y < bmp_info_header.height; ++y) {
                    inp.read((char*)(data.data() + row_stride * y), row_stride);
                    inp.read((char*)padding_row.data(), padding_row.size());
                }
                file_header.file_size += static_cast<uint32_t>(data.size()) + bmp_info_header.height * static_cast<uint32_t>(padding_row.size());
            }
        }
        else {
            throw std::runtime_error("не удается открыть входной файл изображения");
        }
    }

    BMP(int32_t width, int32_t height, bool has_alpha = true) {
        if (width <= 0 || height <= 0) {
            throw std::runtime_error("ширина и высота изображения должны быть положительными числами");
        }

        bmp_info_header.width = width;
        bmp_info_header.height = height;
        if (has_alpha) {
            bmp_info_header.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);

            bmp_info_header.bit_count = 32;
            bmp_info_header.compression = 3;
            row_stride = width * 4;
            data.resize(row_stride * height);
            file_header.file_size = file_header.offset_data + data.size();
        }
        else {
            bmp_info_header.size = sizeof(BMPInfoHeader);
            file_header.offset_data = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

            bmp_info_header.bit_count = 24;
            bmp_info_header.compression = 0;
            row_stride = width * 3;
            data.resize(row_stride * height);

            uint32_t new_stride = make_stride_aligned(4);
            file_header.file_size = file_header.offset_data + static_cast<uint32_t>(data.size()) + bmp_info_header.height * (new_stride - row_stride);
        }
    }

    void write(const char* fname) {
        std::ofstream of{ fname, std::ios_base::binary };
        if (of) {
            if (bmp_info_header.bit_count == 32) {
                write_headers_and_data(of);
            }
            else if (bmp_info_header.bit_count == 24) {
                if (bmp_info_header.width % 4 == 0) {
                    write_headers_and_data(of);
                }
                else {
                    uint32_t new_stride = make_stride_aligned(4);
                    std::vector<uint8_t> padding_row(new_stride - row_stride);

                    write_headers(of);

                    for (int y = bmp_info_header.height - 1; y >= 0; --y) {
                        of.write((const char*)(data.data() + row_stride * y), row_stride);
                        of.write((const char*)padding_row.data(), padding_row.size());
                    }
                }
            }
            else {
                throw std::runtime_error("программа может обрабатывать только bmp-файлы с 24 или 32 битами на пиксель");
            }
        }
        else {
            throw std::runtime_error("не удается открыть выходной файл изображения");
        }
    }

    void fill_region(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        if (x0 + w > static_cast<uint32_t>(bmp_info_header.width) || y0 + h > static_cast<uint32_t>(bmp_info_header.height)) {
            throw std::runtime_error("область не помещается в изображение");
        }

        uint32_t channels = bmp_info_header.bit_count / 8;
        for (uint32_t y = y0; y < y0 + h; ++y) {
            for (uint32_t x = x0; x < x0 + w; ++x) {
                data[channels * (y * bmp_info_header.width + x) + 0] = B;
                data[channels * (y * bmp_info_header.width + x) + 1] = G;
                data[channels * (y * bmp_info_header.width + x) + 2] = R;
                if (channels == 4) {
                    data[channels * (y * bmp_info_header.width + x) + 3] = A;
                }
            }
        }
    }

    void set_pixel(uint32_t x0, uint32_t y0, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        if (x0 >= static_cast<uint32_t>(bmp_info_header.width) || y0 >= static_cast<uint32_t>(bmp_info_header.height) || x0 < 0 || y0 < 0) {
            throw std::runtime_error("точка находится за пределами изображения");
        }

        uint32_t channels = bmp_info_header.bit_count / 8;
        data[channels * (y0 * bmp_info_header.width + x0) + 0] = B;
        data[channels * (y0 * bmp_info_header.width + x0) + 1] = G;
        data[channels * (y0 * bmp_info_header.width + x0) + 2] = R;
        if (channels == 4) {
            data[channels * (y0 * bmp_info_header.width + x0) + 3] = A;
        }
    }

    void draw_rectangle(uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint8_t B, uint8_t G, uint8_t R, uint8_t A, uint8_t line_w) {
        if (x0 + w > static_cast<uint32_t>(bmp_info_header.width) || y0 + h > static_cast<uint32_t>(bmp_info_header.height)) {
            throw std::runtime_error("прямоугольник не помещается в изображение");
        }

        fill_region(x0, y0, w, line_w, B, G, R, A);
        fill_region(x0, y0 + h - line_w, w, line_w, B, G, R, A);
        fill_region(x0, y0, line_w, h, B, G, R, A);
        fill_region(x0 + w - line_w, y0, line_w, h, B, G, R, A);
    }

private:
    uint32_t row_stride{ 0 };

    void write_headers(std::ofstream& of) {
        of.write((const char*)&file_header, sizeof(file_header));
        of.write((const char*)&bmp_info_header, sizeof(bmp_info_header));
        if (bmp_info_header.bit_count == 32) {
            of.write((const char*)&bmp_color_header, sizeof(bmp_color_header));
        }
    }

    void write_headers_and_data(std::ofstream& of) {
        write_headers(of);
        of.write((const char*)data.data(), data.size());
    }

    uint32_t make_stride_aligned(uint32_t align_stride) {
        uint32_t new_stride = row_stride;
        while (new_stride % align_stride != 0) {
            new_stride++;
        }
        return new_stride;
    }

    void check_color_header(BMPColorHeader& bmp_color_header) {
        BMPColorHeader expected_color_header;
        if (expected_color_header.red_mask != bmp_color_header.red_mask ||
            expected_color_header.blue_mask != bmp_color_header.blue_mask ||
            expected_color_header.green_mask != bmp_color_header.green_mask ||
            expected_color_header.alpha_mask != bmp_color_header.alpha_mask) {
            throw std::runtime_error("неожиданный формат цветной маски! программа может обрабатывать только формат BGRA");
        }
        if (expected_color_header.color_space_type != bmp_color_header.color_space_type) {
            throw std::runtime_error("неожиданный тип цветового пространства! программа может обрабатывать только sRGB цветовое пространство");
        }
    }
};