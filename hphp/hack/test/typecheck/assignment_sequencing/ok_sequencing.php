<?hh // strict

// Test a bunch of sequencing things that don't cause errors.

class A {
  public int $x;
  public function __construct(int $y) {
    $this->x = $y;
  }
  public function foo(A $z): void {
    print ($this->x);
  }
}

// OK because constructors and function calls are sequenced
function test1(): void {
  $x = new A(0);
  // This should be fine, although somewhat silly.
  $x->foo($x = new A(1));
}

// OK because arrays are sequenced
function test2(): array<int> {
  return array($x = 0, $x = 1, $x = 2);
}

// OK because $x is unused...
function test3(): string {
  return ($x = 'lol').($x = 'wut');
}

// OK because short circuiting ops are sequenced
function test4(int $y): int {
  $y === 0 && $y = 10;
  return $y;
}

function test5(array<int> $a): int {
  list($x, $y, $z) = $a;
  return $x;
}
function test6(array<array<int>> $a): int {
  list($x, $y, list($z, $w)) = $a;
  return $z;
}

function test7(array<array<int>> $a): int {
  $x = 0;
  list($x, $y, $z) = $a[$x];
  return $x;
}
