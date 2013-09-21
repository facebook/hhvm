<?php
/* Prototype  : int array_unshift(array $array, mixed $var [, mixed ...])
 * Description: Pushes elements onto the beginning of the array
 * Source code: ext/standard/array.c
*/

/*
 * Testing the functionality of array_unshift() by passing 
 * an object to the $var argument
*/

echo "*** Testing array_unshift() : Passing object to \$var argument ***\n";

// simple class with a variable and method
class SimpleClass
{
  public $var1 = 1;
  public function fun1() {
    return $var1;
  }
}

// class without members
class EmptyClass
{
}

// abstract class
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
    echo "defined in child";
  }
}

// class with final method
class FinalClass
{
  private $var4;
  final function finalMethod() {
    echo "This function can't be overloaded";
  }
}

// class with static members
class StaticClass
{
  static $var5 = 2;
  public static function staticMethod() {
    echo "This is a static method";
  }
}

// array to be passed to $array argument
$array = array('f' => "first", "s" => 'second', 1, 2.222);

// array containing different types of objects as elements
$vars = array(
  new SimpleClass(),
  new EmptyClass(),
  new ChildClass(),
  new FinalClass(),
  new StaticClass()
);

// loop through the various elements of $arrays to check the functionality of array_unshift
$iterator = 1;
foreach($vars as $var) {
  echo "-- Iteration $iterator --\n";

  /* with default argument */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift($temp_array, $var) );

  // dump the resulting array
  var_dump($temp_array);

  /* with optional arguments */
  // returns element count in the resulting array after arguments are pushed to
  // beginning of the given array
  $temp_array = $array;
  var_dump( array_unshift($temp_array, $var, "hello", 'world') );

  // dump the resulting array
  var_dump($temp_array);
  $iterator++;
}

echo "Done";
?>