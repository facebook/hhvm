<?hh

abstract class BaseClass {
  public abstract function __construct(string $x);
}

class ChildClass extends BaseClass {
  public function __construct(string $y) {
    parent::__construct('string!');
  }
}
