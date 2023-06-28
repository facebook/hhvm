<?hh

class Foo {
  private $x = Vector { 1, 2, 3 };

  public function heh() :mixed{
    $y = $this->x;
    for ($i = 0; $i < 10; ++$i) {
      $y->add($i);
    }
  }
}


<<__EntryPoint>>
function main_private_props_008() :mixed{
(new Foo)->heh();
}
