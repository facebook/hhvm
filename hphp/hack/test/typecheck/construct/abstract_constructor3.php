<?hh

abstract class C {
  public abstract function __construct();
}

class D extends C {}

class E extends D {
  public function __construct() {}
}
