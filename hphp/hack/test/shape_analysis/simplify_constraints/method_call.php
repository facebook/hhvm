<?hh

class C {
  public function f(string $s, dict<string,mixed> $d, int $i): void {}
}

function main(C $c): void {
  $d1 = dict['a' => 42];
  $d2 = dict['w' => 'hi'];
  $c->f('apple', $d1, 42);
}
