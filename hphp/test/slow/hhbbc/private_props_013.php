<?hh

class A {
  private $x = array();
  private $y = "string";

  public function heh(int $i) {
    $this->x = [array()];
    $this->x[0][3]->foo = [$i];
    return $this;
  }
  public function getY() { return $this->y; }
  public function getX() { return $this->x; }
}

function main() {
  $a = new A;
  var_dump($a->heh(0));
  var_dump($a->getY());
  var_dump($a->getX());
}

<<__EntryPoint>>
function main_private_props_013() {
main();
}
