<?hh // strict

class A {
  <<__Rx>>
  public function __construct(public int $x) {}
  <<__Rx, __MaybeMutable>>
  public function f(): int {
    // OK to pass this as maybe mutable
    // OK to call maybe mutable method
    return $this->x + f($this) + $this->f1();
  }
  <<__Rx, __MaybeMutable>>
  public function f1(): int {
    // OK to read field value
    return $this->x;
  }
}

<<__Rx>>
function f(<<__MaybeMutable>>A $a): int {
  // OK to read field value
  // OK to call maybe mutable method
  // OK to pass as maybe mutable parameter
  return $a->x + $a->f1() + f($a);
}

<<__Rx>>
function g(): void {
  $a = \HH\Rx\mutable(new A(10));
  // OK to pass mutable as maybe mutable
  $v1 = f($a);
  // OK to call maybe mutable method on mutable instance
  $v2 = $a->f();

  $a1 = \HH\Rx\freeze($a);
  // OK to pass immutable as maybe mutable
  $v3 = f($a1);
  // OK to call maybe mutable method on immutable instance
  $v4 = $a1->f();
}
