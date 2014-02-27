<?hh

class Foo {
  private $foo = array(1,2,3);
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

main();
