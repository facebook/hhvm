//// file1.php
<?hh // strict


class C {
  public readonly int $p1 = 0;
  public int $p2 = 0;
  public readonly function f1(readonly mixed $x1, mixed $x2): readonly mixed {
    return $x1;
  }
  public function f2(): void {}
}

function f1(readonly mixed $x1, mixed $x2): readonly mixed {
  return $x1;
}

function f2(): void {}

//// file2.php
<?hh

<<__EntryPoint>>
function main() : void {
  (new ReflectionParameter('C::f1', 'x1'))->isReadonly();
  (new ReflectionParameter('C::f1', 'x2'))->isReadonly();

  (new ReflectionProperty('C', 'p1'))->isReadonly();
  (new ReflectionProperty('C', 'p2'))->isReadonly();

  (new ReflectionFunction('f1'))->returnsReadonly();
  (new ReflectionFunction('f2'))->returnsReadonly();

  (new ReflectionMethod('C::f1'))->returnsReadonly();
  (new ReflectionMethod('C::f2'))->returnsReadonly();

  (new ReflectionMethod('C::f1'))->isReadonly();
  (new ReflectionMethod('C::f2'))->isReadonly();
}
