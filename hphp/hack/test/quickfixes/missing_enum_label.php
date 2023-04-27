<?hh

enum class E: mixed {
  int FOO = 42;
}

function foo(): void {
  $x = E#BOO;
}
