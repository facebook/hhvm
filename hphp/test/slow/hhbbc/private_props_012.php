<?hh

class X {
  private $x    = array();
  private $prop = "string";

  public function foo() {
    $this->x[]->prop = 12;
  }
  public function bar() {
    return $this->prop;
  }
};

function main() {
  $x = new X;
  var_dump($x->foo());
  var_dump($x->bar());
}

main();

