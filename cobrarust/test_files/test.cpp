#include "stdio.h" // "stdio.h"
#include <iostream>

void fn(int* r) {
  printf("%d", *r);
}

int main (int argc, char *argv[]) {
  static int* tushars = (int*)malloc(sizeof(int) * 10);
  int var = 0;
  int* ptr = 0;
  // int* ptrvar = &var;
  std::cin >> var;
  argv[var] = "1";
  return 0;
  char* tt = "this is a long one I think";
  char dst[10];
  strcpy(dst, tt);
}
