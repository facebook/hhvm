<?hh

trait Awesome {
  abstract public function __construct();
}

class C {
  use Awesome;
  public function __construct() {}
}
