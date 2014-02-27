<?hh

class A {
  private $x = array(1,2,3);
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
main();
