#include <cstdio> // "stdio.h"
#include <iostream>

void fn(int* r) {
  printf("%d", *r);
}

int main (int argc, char *argv[]) {
  char* tt = "this is a long one I think";
  char dst[10];
  strcpy(dst, tt);
  scanf("%s", dst);
}
