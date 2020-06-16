#include "globals.hpp"
// #include "read_bin_file.cpp"
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <math.h>
#include <vector>

#include "colorspace/ColorSpace.h"
#include "colorspace/Conversion.h"
#include "super_pixel.cpp"
#include <blaze/Math.h>

using blaze::DynamicMatrix;

int main() {

  std::ifstream file("assets/orange.bin", std::ios::binary);

  // Stop eating new lines in binary mode!!!
  file.unsetf(std::ios::skipws);

  // get its size:
  std::streampos fileSize;

  file.seekg(0, std::ios::end);
  fileSize = file.tellg();
  file.seekg(0, std::ios::beg);

  if (fileSize == -1) {
    std::cout << "file size is -1" << std::endl;
    std::exit(1);
  }

  // reserve capacity
  std::vector<uint8_t> vec(fileSize);
  printf("file size: %d\n", fileSize);

  int i = 0;
  while (!file.eof()) {
    file.read((char *)&vec[i], 1);
    i++;
  }
  printf("file read until end %d\n", i);
  printf("vector size %d\n", vec.size());

  file.close();

  std::cout << "running..." << std::endl;

  // std::vector<uint8_t> *imageFileData = readFile("assets/orange.bin");
  width = 320;
  height = 249;

  std::cout << "image width: " << width << std::endl;
  std::cout << "image height: " << height << std::endl;

  verboseMode = true;
  superpixel(vec);
  // superpixel(&imageFileData);

  // delete imageFileData;
  std::cout << "end of program" << std::endl;

  std::ofstream fout;
  fout.open("superpixel.bin", std::ios::binary | std::ios::out);
  for (int ri = 0; ri < height; ri++) {
    for (int ci = 0; ci < width; ci++) {

      const int cci = clusters[ri][ci];

      ColorSpace::Rgb rgb;
      ColorSpace::Lab lab(centers[cci][2], centers[cci][3], centers[cci][4]);
      lab.To<ColorSpace::Rgb>(&rgb);
      int r = rgb.r;
      int g = rgb.g;
      int b = rgb.b;
      int a = 255;

      fout.write((char *)&r, 1);
      fout.write((char *)&g, 1);
      fout.write((char *)&b, 1);
      fout.write((char *)&a, 1);
    }
  }
  fout.close();

  return 0;
}
