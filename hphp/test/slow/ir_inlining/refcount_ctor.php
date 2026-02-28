<?hh

class Obj {
  public function __construct() {
    $this->uniqueVar = "a string";
  }
  private $uniqueVar;
}

function test() :mixed{
  new Obj();
  echo "done\n";
}


<<__EntryPoint>>
function main_refcount_ctor() :mixed{
test();
}
