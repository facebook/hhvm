<?hh

/**
 * Doc comment on a function generator
 */
function foo() {
  yield null;
}

class C {
  /**
   * Doc comment on a method generator
   */
  public function bar() {
    yield null;
  }
}

<<__EntryPoint>>
function main_1343() {
$rf = new ReflectionFunction('foo');
var_dump($rf->getDocComment());
$rm = new ReflectionMethod('C','bar');
var_dump($rm->getDocComment());
}
