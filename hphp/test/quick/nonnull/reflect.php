<?hh

class Foo {
  const type T = nonnull;
  function bar(nonnull $a, int $b, Foo::T $c): nonnull {
    return $a;
  }
}

function foobar(vec<nonnull> $x): nonnull {
  return $x;
}

function dump($x) :mixed{
  var_dump($x->getReturnType()->__toString());
  foreach ($x->getParameters() as $param) {
    var_dump($param->getType()->__toString());
  }
}

<<__EntryPoint>> function main(): void {
  echo "\nReflectionMethod:\n";
  dump(new ReflectionMethod('Foo::bar'));
  echo "\nReflectionFunction:\n";
  dump(new ReflectionFunction('foobar'));
}
