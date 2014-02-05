<?hh

class Foo {
  private $foo = array(1,2,3);

  public function f() {
    return function () {
      yield 1;
      yield 2;
      yield 3;
      $this->foo = null;
    };
  }

  public function getter() { return $this->foo; }
}

function main() {
  $foo = new Foo();
  var_dump($foo->getter());
  $f = $foo->f();
  var_dump($foo->getter());
  foreach ($f() as $k => $v) {
    var_dump($foo->getter());
  }
  var_dump($foo->getter());
}

main();
