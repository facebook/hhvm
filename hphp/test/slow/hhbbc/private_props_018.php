<?hh

class StringElem {
  private $x = "this is a string";

  public function foo() :mixed{
    return $this->x[3][0];
  }
  public function bar() :mixed{
    return $this->x;
  }
}
function main() :mixed{
  $x = new StringElem;
  var_dump($x->foo());
  var_dump($x->bar());
}


<<__EntryPoint>>
function main_private_props_018() :mixed{
;

main();
}
