<?hh

class Bar {
  public $x = array(1,2,3);
}

class Whatever {
  private $x;

  public function __construct() {
    $this->x = new Bar();
  }

  public function hey() {
    $this->x->x = 123;
    return $this->x->x;
  }
}

function main() {
  var_dump((new Whatever(0))->hey());
}

main();
