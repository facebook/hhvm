<?hh

class Foo {
  private $foo = vec[1,2,3];
  private $bar = "foofoo";

  public function __construct(string $k) {
    $this->{$k}[0] = 2;
  }

  public function getFoo() :mixed{ return $this->foo; }
  public function getBar() :mixed{ return $this->bar; }
}

function main() :mixed{
  $a = new Foo('foo');
  var_dump($a->getFoo());
  var_dump($a->getBar());
}


<<__EntryPoint>>
function main_private_props_015() :mixed{
main();
}
