
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#define BYTE uint8_t

std::vector<BYTE> *readFile(const char *filename) {
  // open the file:
  std::ifstream file(filename, std::ios::binary);

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
  std::vector<BYTE> *vec = new std::vector<BYTE>(fileSize);

  int i = 0;
  while (!file.eof()) {
    file.read((char *)&(*vec)[i++], 1);
  }
  printf("file read until end %d\n", i);
  printf("vector size %d\n", vec->size());

  file.close();

  return vec;
}
