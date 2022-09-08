<?hh

enum class VeryLongClassName: mixed {
  int f = 1;
  int foobar = 1;
}

function foo(): void {
  $x = VeryLongClassName#foobarr;
}
