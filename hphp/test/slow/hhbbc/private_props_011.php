<?hh

class A {
  private $x = dict[];
  private $y = "string";

  public function heh(int $i) :mixed{
    $this->x[$i] = $i;
    return $this;
  }
  public function getY() :mixed{ return $this->y; }
  public function getX() :mixed{ return $this->x; }
}

function main() :mixed{
  $a = new A;
  var_dump($a->heh(0));
  var_dump($a->getY());
  var_dump($a->getX());
}

<<__EntryPoint>>
function main_private_props_011() :mixed{
main();
}
