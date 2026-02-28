<?hh

class Foo {
  const type T = nothing;
  function bar(nothing $a, int $b, Foo::T $c): nothing {
    return $a;
  }
}

function foobar(vec<nothing> $x): nothing {
  return $x[0];
}

function dump($x) :mixed{
  var_dump((string)$x->getReturnType());
  foreach ($x->getParameters() as $param) {
    var_dump($param->isArray());
    var_dump((string)$param->getType());
  }
}

<<__EntryPoint>> function main(): void {
  echo "\nReflectionMethod:\n";
  dump(new ReflectionMethod('Foo::bar'));
  echo "\nReflectionFunction:\n";
  dump(new ReflectionFunction('foobar'));
}
