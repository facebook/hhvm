<?hh

class Foo {
  static int $x = 0;
}

<<__NoContext>>
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
  include 'implicit.inc';

  f();
  try {
    ClassContext::start(new C, f<>);
    echo "Failure2!\n";
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }
}
