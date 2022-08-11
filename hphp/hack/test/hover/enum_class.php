<?hh

enum class Foo: mixed {
  /** Hello */
  int BAR = 1;
}

function demo(): void {
  $value = Foo#BAR;
  //           ^ hover-at-caret
}
