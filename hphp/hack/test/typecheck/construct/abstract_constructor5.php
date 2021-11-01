<?hh

trait Awesome {
  abstract public function __construct();
}

class C {
  use Awesome;
}

class D extends C {
  public function __construct() {}
}
