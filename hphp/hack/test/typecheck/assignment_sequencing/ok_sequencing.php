<?hh // strict

// Test a bunch of sequencing things that don't cause errors.

class A {
  public int $x;
  public function __construct(int $y) {
    $this->x = $y;
  }
  public function foo(A $z): void {
    print($this->x);
  }
}


function test1(array<int> $a): int {
  list($x, $y, $z) = $a;
  return $x;
}
function test2(array<array<int>> $a): int {
  list($x, $y, list($z, $w)) = $a;
  return $z;
}

function test3(array<array<int>> $a): int {
  $x = 0;
  list($x, $y, $z) = $a[$x];
  return $x;
}
