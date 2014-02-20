<?hh

class B{}

class Foo {
  private $x;
  private $yo;

  public function __construct() {
    $this->x = new B();
    $GLOBALS['foo'][12]->bar = 2;
  }

  public function getX() { return $this->x; }
}

function main() {
  $f = new Foo();
  var_dump($f->getX());
}

main();
