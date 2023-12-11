<?hh

class Bar {
  public $x = vec[1,2,3];
}

class Whatever {
  private $x;

  public function __construct() {
    $this->x = new Bar();
  }

  public function hey() :mixed{
    $this->x->x = 123;
    return $this->x->x;
  }
}

function main() :mixed{
  var_dump((new Whatever(0))->hey());
}


<<__EntryPoint>>
function main_private_props_009() :mixed{
main();
}
