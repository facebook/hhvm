<?hh

type AMixedType = mixed;

type ADictType = dict<string, AMixedType>;

class CDictWithUnsafeType {
  const type TDict = dict<string, AMixedType>;
}

class Foo {
  const type TDict = dict<int, int>;
  function bar(dict<string, int> $a, int $b, Foo::Tdict $c): dict {
    return dict[];
  }
}

function foobar(dict<int, string> $x, dict<arraykey, Foo> $y): dict<int, int> {
  return dict[];
}

function dump($x) :mixed{
  var_dump((string)$x->getReturnType());
  foreach ($x->getParameters() as $param) {
    var_dump($param->isArray());
    var_dump((string)$param->getType());
  }
}

function main() :mixed{
  echo("\nReflectionMethod:\n");
  dump(new ReflectionMethod('Foo::bar'));

  echo("\nReflectionFunction:\n");
  dump(new ReflectionFunction('foobar'));

  echo("\nFoo::TDict type_structure:\n");
  var_dump(type_structure(Foo::class, 'TDict'));
  var_dump(gettype(dict[]));

  echo("\nCDictWithUnsafeType::TDict type_structure:\n");
  var_dump(type_structure(CDictWithUnsafeType::class, 'TDict'));

  echo("\nADictType type_structure:\n");
  var_dump(type_structure(ADictType::class));
}
<<__EntryPoint>>
function main_entry(): void {

  main();
}
