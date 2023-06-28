<?hh

class GetterInternal {
  private $foo;

  public function __construct() {
    $this->foo = "asd";
  }

  public function doit() :mixed{
    return $this->getFoo() . "asd";
  }

  public function getFoo() :mixed{
    return $this->foo;
  }
}

function test10() :mixed{
  $k = new GetterInternal();
  $k->doit();
}


<<__EntryPoint>>
function main_getter_internal() :mixed{
test10();
}
