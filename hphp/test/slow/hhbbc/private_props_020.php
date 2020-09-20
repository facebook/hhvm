<?hh

class B{}

class Foo {
  private $x;
  private $yo;

  public function __construct() {
    $this->x = new B();
    \HH\global_set('foo', darray[12 => new stdClass()]);
    \HH\global_get('foo')[12]->bar = 2;
  }

  public function getX() { return $this->x; }
}

function main() {
  $f = new Foo();
  var_dump($f->getX());
}


<<__EntryPoint>>
function main_private_props_020() {
main();
}
