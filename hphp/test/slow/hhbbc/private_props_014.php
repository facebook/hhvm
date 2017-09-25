<?hh

class A {
  private $x = array();
  private $y = "string";

  public function heh(int $i) {
    var_dump($this->x->x->hey++);
    return $i;
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
main();
