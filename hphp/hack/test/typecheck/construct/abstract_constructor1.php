<?hh

abstract class C {
  public abstract function __construct();
}

class D extends C {
  public function __construct() {}
}
