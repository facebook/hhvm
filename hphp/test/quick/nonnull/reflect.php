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

function dump($x) {
  var_dump((string)$x->getReturnType());
  foreach ($x->getParameters() as $param) {
    var_dump($param->isArray());
    var_dump((string)$param->getType());
  }
}

function main() {
  echo "\nReflectionMethod:\n";
  dump(new ReflectionMethod('Foo::bar'));
  echo "\nReflectionFunction:\n";
  dump(new ReflectionFunction('foobar'));
}

main();
