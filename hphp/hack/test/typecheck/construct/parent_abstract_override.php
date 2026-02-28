<?hh

abstract class BaseClass {
  public abstract function __construct();
}

final class ChildClass extends BaseClass {
  public function __construct() {
    parent::__construct();
  }
}
