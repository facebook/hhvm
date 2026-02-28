<?hh

class Lal {
}

class Foo {
  private $x = null;

  public function __construct() {
    $this->x = new Lal;
  }

  public function sup() :mixed{
    $this->x[] = 2;
  }
}

function main() :mixed{
  $x = new Foo;
  $x->sup();
}


<<__EntryPoint>>
function main_not_supported_exception() :mixed{
main();
}
