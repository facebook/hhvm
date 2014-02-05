<?hh

class Foo {
  private $foo;
  private $bar = "foofoo";

  public function __construct(string $k) {
    $this->{$k}->prop = 2;
  }

  public function getFoo() { return $this->foo; }
  public function getBar() { return $this->bar; }
}

function main() {
  $a = new Foo('foo');
  var_dump($a->getFoo());
  var_dump($a->getBar());
  var_dump($a);
}

main();
