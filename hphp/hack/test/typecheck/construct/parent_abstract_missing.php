<?hh

abstract class BaseClass {}

final class ChildClass extends BaseClass {
  public function __construct() {
    parent::__construct();
  }
}
