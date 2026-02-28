<?hh

class A {
  private $x = vec[1,2,3];
  private $y = "string";

  public function heh(int $i) :mixed{
    $this->x[$i] = $i;
    return $this;
  }
  public function getY() :mixed{ return $this->y; }
}

function main() :mixed{
  $a = new A;
  var_dump($a->heh(0));
  var_dump($a->getY());
}

<<__EntryPoint>>
function main_private_props_010() :mixed{
main();
}
