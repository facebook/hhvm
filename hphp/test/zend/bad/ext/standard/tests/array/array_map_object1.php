<?php
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Testing array_map() for object functionalities:
 *   1) simple class with variable and method
 *   2) class without members
 *   3) class with only one method and no variable
 *   4) abstract and child class
 *   5) class with static and final members
 *   6) interface and implemented class
 */
echo "*** Testing array_map() : object functionality ***\n";

echo "-- simple class with public variable and method --\n";
class SimpleClass
{
  public $var1 = 1;
  public function square($n) {
    return $n * $n;
  }
}
function test($cb, $args) {
  echo join('::', $cb) . "\n";
  var_dump(array_map($cb, $args));
}
test(array('SimpleClass', 'square'), array(1, 2));

echo "\n-- simple class with private variable and method --\n";
class SimpleClassPri
{
  private $var1 = 10;
  private function add($n) {
    return $var + $n;
  }
}
test(array('SimpleClassPri', 'add'), array(1));

echo "\n-- simple class with protected variable and method --\n";
class SimpleClassPro
{
  protected $var1 = 5;
  protected function mul($n) {
    return $var1 * $n;
  }
}
test(array('SimpleClassPro', 'mul'), array(2));

echo "\n-- class without members --\n";
class EmptyClass
{
}
test(array('EmptyClass'), array(1, 2));

echo "\n-- abstract class --\n";
abstract class AbstractClass
{
  protected $var2 = 5;
  abstract function emptyFunction();
}

// class deriving the above abstract class
class ChildClass extends AbstractClass
{
  private $var3;
  public function emptyFunction() {
    echo "defined in child\n";
  }
}
test(array('ChildClass', 'emptyFunction'), array(1, 2));

echo "\n-- class with final method --\n";
class FinalClass
{
  private $var4;
  final function finalMethod() {
    echo "This function can't be overloaded\n";
  }
}
test(array('FinalClass', 'finalMethod'), array(1, 2));

echo "\n-- class with static members --\n";
class StaticClass
{
  static $var5 = 2;
  public static function square($n) {
    return ($n * $n);
  }
  private static function cube($n) {
    return ($n * $n * $n);
  }
  protected static function retVal($n)  {
    return array($n);
  }
}
test(array('StaticClass', 'square'), array(1, 2));
test(array('StaticClass', 'cube'), array(2));
test(array('StaticClass', 'retVal'), array(3, 4));

echo "-- class implementing an interface --\n";
interface myInterface
{
  public function toImplement();
}
class InterClass implements myInterface
{
  public static function square($n) {
    return ($n * $n);
  }
  public function toImplement() {
    return 1;
  }
}
test(array('InterClass', 'square'), array(1, 2));

?>
===DONE===
<?php exit(0); ?>