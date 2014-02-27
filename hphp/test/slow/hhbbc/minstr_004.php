<?hh

class Foo {
  private $x;

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

main();
