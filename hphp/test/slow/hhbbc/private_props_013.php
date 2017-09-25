<?hh

class A {
  private $x = array();
  private $y = "string";

  public function heh(int $i) {
    $this->x[][3]->foo[] = $i;
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
main();
