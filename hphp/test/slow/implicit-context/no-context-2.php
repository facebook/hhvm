<?hh

include 'implicit.inc';

class Foo {
  static int $x = 0;
}

<<__Memoize, __NoContext>>
function f() {
  if (Foo::$x === 0) {
    Foo::$x = 1;
    echo "Good!\n";
    return;
  }
  echo "Failure!\n";
}

<<__EntryPoint>>
function main() {
  f();
  try {
    ClassContext::start(new C, fun('f'));
    echo "Failure2!\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
