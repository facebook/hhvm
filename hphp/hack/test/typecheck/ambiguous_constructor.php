<?hh // strict
trait T1 {
  public function __construct() {}
}
trait T2 {
  public function __construct() {}
}

class C {
  use T1, T2;
}
