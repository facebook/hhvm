<?hh

class SetM {
  private $x;

  public function __construct() {
    $this->x = "asdasd";
  }

  public function clearX() :mixed{
    $this->x = null;
  }
}

function main() :mixed{
  $x = new SetM();
  $x->clearX();
}


<<__EntryPoint>>
function main_setm() :mixed{
main();
}
