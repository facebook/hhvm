<?hh

class Foo {
  static public function bar(): int {
    return 1;
  }

  static public function bar_with_generic<T>(): int {
    return 1;
  }

  static public function bar_with_generics<T, Ta, Tb, Tc>(): int {
    return 1;
  }
}

function baz(): int {
  return 1;
}

function baz_with_generic<T>(): int {
  return 1;
}

function baz_with_generics<T, Ta, Tb, Tc>(): int {
  return 1;
}

function func1($p0 = Foo::bar<>) :mixed{}
function func2($p0 = Foo::bar_with_generic<int>) :mixed{}
function func3($p0 = Foo::bar_with_generics<int, string, int, string>) :mixed{}
function func4($p0 = baz<>) :mixed{}
function func5($p0 = baz_with_generic<int>) :mixed{}
function func6($p0 = baz_with_generics<int, string, int, string>) :mixed{}

<<__EntryPoint>>
function main() :mixed{
  $funcs = vec[
    'func1',
    'func2',
    'func3',
    'func4',
    'func5',
    'func6',
  ];

  foreach($funcs as $func) {
    $x = new ReflectionFunction($func);
    var_dump($x->getParameters());
  }
}
