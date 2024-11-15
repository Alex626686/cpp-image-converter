#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <cassert>
#include <iostream>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {


    // функция вычисления отступа по ширине
    static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }


PACKED_STRUCT_BEGIN BitmapInfoHeader{
    BitmapInfoHeader() = default;
    BitmapInfoHeader(int stride, int width, int height)
        : width_(width), height_(height) {
        bytes_in_data = stride * height;
    }
    uint32_t header_size = 40;
    int32_t width_ = {};
    int32_t height_ = {};
    uint16_t plane_amt = 1;
    uint16_t bit_on_pixel = 24;
    uint32_t compression_type = 0;
    uint32_t bytes_in_data = {};
    int32_t hor_resolution = 11811;
    int32_t vert_resolution = 11811;
    int32_t color_amt = 0;
    int32_t sign_color_amt = 0x1000000;
}
PACKED_STRUCT_END


PACKED_STRUCT_BEGIN BitmapFileHeader{
    BitmapFileHeader() = default;
    BitmapFileHeader(int stride, int height) {
        sum_size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + stride * height;
    }
    char signature[2] = {'B', 'M'};
    uint32_t sum_size = {};
    uint32_t Reserved = 0;
    uint32_t indent = 54;
}
PACKED_STRUCT_END

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    const int stride = GetBMPStride(image.GetWidth());

    BitmapFileHeader file_header{ stride, image.GetHeight() };
    out.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    BitmapInfoHeader info_header{ stride, image.GetWidth(), image.GetHeight() };
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));

    vector<char> buff(stride);
    for (int y = image.GetHeight() - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < image.GetWidth(); ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(reinterpret_cast<const char*>(buff.data()), buff.size());
    }

    return true;

}

//bool HeaderCheck()

// напишите эту функцию
Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    if (!ifs) {
        return Image;
    }
    BitmapFileHeader bfh;
    ifs.read(reinterpret_cast<char*>(&bfh), sizeof(BitmapFileHeader));
    if (!bfh.signature == char[2]({ 'B', 'M' })) {
        return Image;
    }

    BitmapInfoHeader bih;
    ifs.read(reinterpret_cast<char*>(&bih), sizeof(BitmapInfoHeader));

    int width = bih.width_;
    int height = bih.height_;

    int stride = GetBMPStride(width);
    Image result(stride / 3, height, Color::Black());
    std::vector<char> buff(width * 3);

    for (int y = result.GetHeight() - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), stride);
        for (int x = 0; x < width; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib