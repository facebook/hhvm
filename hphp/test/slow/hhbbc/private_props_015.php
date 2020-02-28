<?hh

class Foo {
  private $foo = varray[1,2,3];
  private $bar = "foofoo";

  public function __construct(string $k) {
    $this->{$k}[0] = 2;
  }

  public function getFoo() { return $this->foo; }
  public function getBar() { return $this->bar; }
}

function main() {
  $a = new Foo('foo');
  var_dump($a->getFoo());
  var_dump($a->getBar());
}


<<__EntryPoint>>
function main_private_props_015() {
main();
}
