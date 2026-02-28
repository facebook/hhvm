<?hh

class C {
  public readonly int $p1;
  public int $p2;
  readonly function f1(readonly mixed $x1, mixed $x2): readonly mixed {
    return $x1;
  }
  function f2() :mixed{}
}

function f1(readonly mixed $x1, mixed $x2): readonly mixed {
  return $x1;
}
function f2() :mixed{}

<<__EntryPoint>>
function main() :mixed{
  var_dump((new ReflectionParameter('C::f1', 'x1'))->isReadonly());
  var_dump((new ReflectionParameter('C::f1', 'x2'))->isReadonly());

  var_dump((new ReflectionProperty('C', 'p1'))->isReadonly());
  var_dump((new ReflectionProperty('C', 'p2'))->isReadonly());

  var_dump((new ReflectionFunction('f1'))->returnsReadonly());
  var_dump((new ReflectionFunction('f2'))->returnsReadonly());

  var_dump((new ReflectionMethod('C::f1'))->returnsReadonly());
  var_dump((new ReflectionMethod('C::f2'))->returnsReadonly());

  var_dump((new ReflectionMethod('C::f1'))->isReadonly());
  var_dump((new ReflectionMethod('C::f2'))->isReadonly());
}
