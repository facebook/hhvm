<?hh
trait TX {
  public function __construct() {}
}
trait T1 {
  use TX;
}
trait T2 {
  public function __construct() {}
}

class C {
  use T1, T2;
}
