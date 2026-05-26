<?hh

class A {}

function get_A_opt(): ?A {
  return new A();
}

function fwhile(): void {
  $x = new A();
  while ($x is nonnull) {
    $x = get_A_opt();
  }
}

function ffor(): void {
  for ($x = new A(); $x is nonnull; $x = get_A_opt()) {
  }
}
