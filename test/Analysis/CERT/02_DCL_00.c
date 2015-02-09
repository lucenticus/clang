// RUN: %clang_cc1 -analyze -analyzer-checker=alpha.cert.DCL_00 -verify %s
void test1() {
  float pi = 3.14159f;  // expected-warning {{DCL00-C}}
  float degrees = 1; // expected-warning {{DCL00-C}}
  float radians;
  /* ... */
  radians = degrees * pi / 180;
}
