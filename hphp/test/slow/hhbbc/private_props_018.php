<?hh

class StringElem {
  private $x = "this is a string";

  public function foo() {
    return $this->x[3][0];
  }
  public function bar() {
    return $this->x;
  }
};

function main() {
  $x = new StringElem;
  var_dump($x->foo());
  var_dump($x->bar());
}

main();

