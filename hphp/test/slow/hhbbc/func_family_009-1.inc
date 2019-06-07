<?hh

abstract class Middle extends Base {
  abstract function abs();

  function concrete_override() {
    $x = parent::concrete_override();
    return null;
  }
}
class Derived extends Middle {
  function abs() {}
}
$x = 1;
