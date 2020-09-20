<?hh //partial

class C {
  public function foo($x, $y): void {
    bar($x, $y);
  }
}

function bar(int $x, string $y): void {}
