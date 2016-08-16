<?hh

class Foo {
  const type Tkeyset = HH\keyset<int>;
  function bar(HH\keyset<int> $a, int $b, Foo::Tkeyset $c): HH\keyset {
    return keyset[];
  }
}

function foobar(HH\keyset<string> $x, HH\keyset<arraykey> $y): HH\keyset<int> {
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
