<?hh

class Foo {
  const type Tkeyset = keyset<int>;
  function bar(keyset<int> $a, int $b, Foo::Tkeyset $c): keyset {
    return keyset[];
  }
}

function foobar(keyset<string> $x, keyset<arraykey> $y): keyset<int> {
  return keyset[];
}

function dump($x) {
  var_dump((string)$x->getReturnType());
  foreach ($x->getParameters() as $param) {
    var_dump($param->isArray());
    var_dump((string)$param->getType());
  }
}

function main() {
  dump(new ReflectionMethod('Foo::bar'));
  dump(new ReflectionFunction('foobar'));

  var_dump(type_structure(Foo::class, 'Tkeyset'));
  var_dump(gettype(keyset[]));
}

main();
