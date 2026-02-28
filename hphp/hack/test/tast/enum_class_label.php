<?hh

enum class MyEnumClass: mixed {
  int Y = 1;
}

function foo(): void {
  MyEnumClass#Y;
}
