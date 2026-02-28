<?hh

class Foo {
  const type Tvec = vec<int>;
  function bar(vec<string> $a, int $b, Foo::Tvec $c): vec {
    return vec[];
  }
}

function foobar(vec<string> $x, vec<Foo> $y): vec<int> {
  return vec[];
}

function dump($x) :mixed{
  var_dump((string)$x->getReturnType());
  foreach ($x->getParameters() as $param) {
    var_dump($param->isArray());
    var_dump((string)$param->getType());
  }
}

<<__EntryPoint>> function main(): void {
  dump(new ReflectionMethod('Foo::bar'));
  dump(new ReflectionFunction('foobar'));

  var_dump(type_structure(Foo::class, 'Tvec'));
  var_dump(gettype(vec[]));
}
