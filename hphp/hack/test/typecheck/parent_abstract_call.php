<?hh

abstract class BaseClass {
  public abstract function XYZ(): bool;
}

class ChildClass extends BaseClass {
  public function XYZ(): bool {
    return parent::XYZ(); // parent's is abstract!
  }
}
