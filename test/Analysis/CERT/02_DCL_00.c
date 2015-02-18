// RUN: %clang_cc1 -analyze -analyzer-checker=alpha.cert.DCL_00 -verify %s
void test1() {
  float pi = 3.14159f;  // expected-warning {{DCL00-C}}
  float degrees = 1; // expected-warning {{DCL00-C}}
  const int degrees_const = 1; // no-warning 
  
  float radians;
  /* ... */
  radians = degrees * pi / 180;
}

void test2(int b) {
  int i = 1;
  int j;
  if (b)
    i = 2;
  else
    i = 3;
  j = 5;
}
