<?hh

class X {
  private $x    = vec[null];
  private $prop = "string";

  public function foo() :mixed{
    try {
      $this->x[0]->prop = 12;
    } catch (Exception $e) {
      return $e->getMessage();
    }
  }
  public function bar() :mixed{
    return $this->prop;
  }
}
function main() :mixed{
  $x = new X;
  var_dump($x->foo());
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_012() :mixed{
;

main();
}
