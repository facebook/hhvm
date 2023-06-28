<?hh

class CGetMTest {
  public function __construct() {
    $this->uniqueVar = "a string";
  }

  public function getX() :mixed{
    // TODO test something that will throw, make sure stack
    // materialization worked.
    return $this->uniqueVar;
  }

  private $uniqueVar;
}

function test5() :mixed{
  $obj = new CGetMTest();
  echo $obj->getX();
  echo "\n";
}

function test9() :mixed{
  // $this is on the stack; can we still handle incref / decref
  // elimination (not right now).
  echo (new CGetMTest())->getX();
  echo "\n";
}


<<__EntryPoint>>
function main_cgetm() :mixed{
test5();
test9();
}
