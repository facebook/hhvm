<?hh // strict

class C implements Stringish {
  public function __toString(): string { return ''; }
}

function test(C $x): void {
  if ($x) {
  }
}
