<?hh

class Foo {}

interface IFoo {
  require extends Foo;
}

class Bar extends Foo implements IFoo {}

function bar() {
  define('BUZ', 42);
  global $g;
  return $g ? new Foo() : new Bar();
}

function foo() {
  define('FIZ', 42);
  $f = bar();

  if ($f !== null && $f is IFoo) {
    return $f;
  }
  return $f;
}

echo "foo";
