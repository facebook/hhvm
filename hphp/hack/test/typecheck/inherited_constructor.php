<?hh

trait T {
  public abstract function __construct();
}

class A {
  final public function __construct() {}
}

class B extends A {
  use T;
}
