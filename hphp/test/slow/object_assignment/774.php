<?hh

class foo {
  public function __construct() {
    $this->val = 1;
  }
  function bar() :mixed{
    echo $this->val;
    $ref2 = $this;
    $ref2->val = 3;
    echo $this->val;
    $x = new foo();
    echo $x->val;
    $ref4 = $x;
    $ref4->val = 5;
    echo $x->val;
  }
  public $val;
}

<<__EntryPoint>>
function main() :mixed{
  $x = new foo();
  $x->bar();
  $ref5 = $x;
  $ref5->val = 6;
  echo $x->val;
}
