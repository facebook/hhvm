<?hh

/** Doc comment here. */
enum class MyEnumClass: int {
  int RED = 1;
  int GREEN = 2;
  int BLUE = 3;
}

function use_it(): void {
  $x = MyEnumClass#RED;
  //       ^ hover-at-caret
}
