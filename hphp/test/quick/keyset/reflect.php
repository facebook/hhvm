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

function dump($x) :mixed{
  var_dump($x->getReturnType()->__toString());
  foreach ($x->getParameters() as $param) {
    var_dump($param->getType()->__toString());
  }
}

<<__EntryPoint>> function main(): void {
  dump(new ReflectionMethod('Foo::bar'));
  dump(new ReflectionFunction('foobar'));

  var_dump(type_structure(Foo::class, 'Tkeyset'));
  var_dump(gettype(keyset[]));
}
