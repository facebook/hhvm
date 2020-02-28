<?hh

class A {
  private $x = varray[];
  private $y = "string";

  public function heh(int $i) {
    $this->x[$i] = $i;
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
function main_private_props_011() {
main();
}
