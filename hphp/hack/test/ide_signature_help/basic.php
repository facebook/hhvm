<?hh

function add(int $a, int $b): int {
  return $a + $b;
}

function test(): void {
  add(1, 2);
  //   ^ at-caret
}
