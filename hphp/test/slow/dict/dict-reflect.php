<?hh

class Foo {
  const type Tdict = dict<int, int>;
  function bar(dict<string, int> $a, int $b, Foo::Tdict $c): dict {
    return dict[];
  }
}

function foobar(dict<int, string> $x, dict<arraykey, Foo> $y): dict<int, int> {
  return dict[];
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

  var_dump(type_structure(Foo::class, 'Tdict'));
  var_dump(gettype(dict[]));
}

main();
