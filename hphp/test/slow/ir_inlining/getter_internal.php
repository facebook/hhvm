<?hh

class GetterInternal {
  private $foo;

  public function __construct() {
    $this->foo = "asd";
  }

  public function doit() {
    return $this->getFoo() . "asd";
  }

  public function getFoo() {
    return $this->foo;
  }
}

function test10() {
  $k = new GetterInternal();
  $k->doit();
}


<<__EntryPoint>>
function main_getter_internal() {
test10();
}
