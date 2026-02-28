<?hh

class Foo {
  static public function rbar<reify T>(): int {
    return 1;
  }
}

function rbaz<reify T>(): int {
  return 1;
}

function func1($p0 = Foo::rbar<int>) :mixed{}
function func2($p0 = rbaz<int>) :mixed{}

// Actual specified function calls
function func3($p0 = Foo::rbar<int>()) :mixed{}
function func4($p0 = rbaz<int>()) :mixed{}

<<__EntryPoint>>
function main() :mixed{
  $funcs = vec[
    'func1',
    'func2',
    'func3',
    'func4',
  ];

  foreach($funcs as $func) {
    try {
    $x = new ReflectionFunction($func);
    var_dump($x->getParameters());
    } catch (Exception $e) {
      var_dump($e->getMessage());
    }
  }
}
