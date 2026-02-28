<?hh

class B{}

class Foo {
  private $x;
  private $yo;

  public function __construct() {
    $this->x = new B();
    \HH\global_set('foo', dict[12 => new stdClass()]);
    \HH\global_get('foo')[12]->bar = 2;
  }

  public function getX() :mixed{ return $this->x; }
}

function main() :mixed{
  $f = new Foo();
  var_dump($f->getX());
}


<<__EntryPoint>>
function main_private_props_020() :mixed{
main();
}
