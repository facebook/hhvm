<?hh

class A {
  private $x = varray[1,2,3];
  private $y = "string";

  public function heh(int $i) {
    $this->x[$i] = $i;
    return $this;
  }
  public function getY() { return $this->y; }
}

function main() {
  $a = new A;
  var_dump($a->heh(0));
  var_dump($a->getY());
}

<<__EntryPoint>>
function main_private_props_010() {
main();
}
