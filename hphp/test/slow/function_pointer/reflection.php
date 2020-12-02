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

function func1($p0 = Foo::bar<>) {}
function func2($p0 = Foo::bar_with_generic<int>) {}
function func3($p0 = Foo::bar_with_generics<int, string, int, string>) {}
function func4($p0 = baz<>) {}
function func5($p0 = baz_with_generic<int>) {}
function func6($p0 = baz_with_generics<int, string, int, string>) {}

<<__EntryPoint>>
function main() {
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
