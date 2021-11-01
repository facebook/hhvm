<?hh

interface I {
  public function __construct();
}

abstract class C implements I {}

class D extends C {
  public function __construct() {}
}
