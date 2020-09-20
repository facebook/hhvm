<?hh

class X {
  private $x    = varray[null];
  private $prop = "string";

  public function foo() {
    try {
      $this->x[0]->prop = 12;
    } catch (Exception $e) {
      return $e->getMessage();
    }
  }
  public function bar() {
    return $this->prop;
  }
}
function main() {
  $x = new X;
  var_dump($x->foo());
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_012() {
;

main();
}
