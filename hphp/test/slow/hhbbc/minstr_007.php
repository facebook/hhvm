<?hh

class A { public function go($x) {} }
class B { public function go(inout $x) {} }

class Foo {
  private $foo = "asd";
  private $bar = 2;

  public function heh($obj, string $heh) {
    $obj->go($this->bar);
    return $heh;
  }

  public function getter() { return $this->foo; }
}

<<__EntryPoint>>
function main_minstr_007() {
  $foo = new Foo;
  $a = new A;
  var_dump($foo->getter());
  var_dump($foo->heh($a, 'str'));
  var_dump($foo->getter());
}
