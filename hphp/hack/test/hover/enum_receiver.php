<?hh

/** Doc comment here. */
enum MyEnum: int {
  RED = 1;
  GREEN = 2;
  BLUE = 3;
}

function use_it(): void {
  $x = MyEnum::RED;
  //       ^ hover-at-caret
}
