// RUN: %clang_cc1 -analyze -analyzer-checker=alpha.different.LabelInsideSwitch -verify %s

void f1() {
  int c = 7;
  switch(c){
  case 1:
    c += 1; break;
  defalt: 			// expected-warning{{Possible misprint: label found inside the switch statement}}
    c -= 1; break;
  }
}
