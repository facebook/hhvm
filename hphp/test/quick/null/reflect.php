<?hh

class Foo {
  const type T = null;
  function bar(null $a, int $b, Foo::T $c): null {
    return $a;
  }
}

function foobar(vec<null> $x): null {
  return $x[0];
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
