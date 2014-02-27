<?hh

class B { }

class Foo {
  private $x;

  public function __construct() {
    $this->x = new B();
  }

  public function getX() { return $this->x; }

  public function weird_dynamic(array $a, array $b, string $c) {
    ${$c}[0] = 2;
    return ${$c}[0];
  }
}

function main() {
  $f = new Foo();
  var_dump($f->getX());
  var_dump($f->weird_dynamic(array(1,2,3), array('one','two','three'), 'b'));
  var_dump($f->weird_dynamic(array(1,2,3), array('one','two','three'), 'a'));
  var_dump($f->getX());
}

main();
