<?hh

class Foo {
  private $x;
  public function __construct() { $this->x = new stdClass(); }
  public function get() { return $this->x; }
  public function set($y) { $this->x->x = $y; }
}

function main() {
  $x = new Foo();
  var_dump($x);
  $x->set(12);
  var_dump($x->get());
  var_dump($x);
}


<<__EntryPoint>>
function main_minstr_004() {
main();
}
