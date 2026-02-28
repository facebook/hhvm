<?hh

class Foo {
  private $foo;
  private $bar = "foofoo";

  public function __construct(string $k) {
    $this->foo = new stdClass();
    $this->{$k}->prop = 2;
  }

  public function getFoo() :mixed{ return $this->foo; }
  public function getBar() :mixed{ return $this->bar; }
}

function main() :mixed{
  $a = new Foo('foo');
  var_dump($a->getFoo());
  var_dump($a->getBar());
  var_dump($a);
}


<<__EntryPoint>>
function main_private_props_016() :mixed{
main();
}
