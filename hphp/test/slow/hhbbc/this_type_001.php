<?hh

class Foo {
  private $x = "string";

  public function getStr() { return $this->x; }

  public function heh() {
    $y = $this;
    for ($i = 0; $i < 10; ++$i) { $y = $this; }
    if (!$y) throw new Exception('x');
    return $y->getStr();
  }
}

function main() {
  $foo = new Foo;
  return $foo->heh();
}

<<__EntryPoint>>
function main_this_type_001() {
echo main() . "\n";
}
