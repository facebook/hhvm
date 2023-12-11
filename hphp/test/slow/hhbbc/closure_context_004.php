<?hh

class Foo {
  private $foo = vec[1,2,3];

  public function f() :mixed{
    return function () {
      yield 1;
      yield 2;
      yield 3;
      $this->foo = null;
    };
  }

  public function getter() :mixed{ return $this->foo; }
}

function main() :mixed{
  $foo = new Foo();
  var_dump($foo->getter());
  $f = $foo->f();
  var_dump($foo->getter());
  foreach ($f() as $k => $v) {
    var_dump($foo->getter());
  }
  var_dump($foo->getter());
}


<<__EntryPoint>>
function main_closure_context_004() :mixed{
main();
}
