<?hh

class NonExistProp {
  private $x;
  public function __construct() {
 $x = "str";
 }
  public function unsetIt() :mixed{
    unset($this->x);
  }
  public function getX() :mixed{
    return $this->x;
  }
}

// TODO: this will need a hopt to enable throw
function thrower() :mixed{
  //var_dump(debug_backtrace());
  throw new Exception("Yo");
}

function test7() :mixed{
  set_error_handler(thrower<>);
  try {
    $obj = new NonExistProp();
    $obj->unsetIt();
    $k = new Dtor();
    echo $obj->getX();
    echo "\n";
  }
 catch (Exception $x) {
}
}


<<__EntryPoint>>
function main_unset() :mixed{
test7();
}
