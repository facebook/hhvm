<?hh

class Foo {
  private $x;
  public function __construct() { $this->x = new stdClass(); }
  public function get() :mixed{ return $this->x; }
  public function set($y) :mixed{ $this->x->x = $y; }
}

function main() :mixed{
  $x = new Foo();
  var_dump($x);
  $x->set(12);
  var_dump($x->get());
  var_dump($x);
}


<<__EntryPoint>>
function main_minstr_004() :mixed{
main();
}
