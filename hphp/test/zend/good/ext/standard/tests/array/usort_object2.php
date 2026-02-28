<?hh
/* Prototype  : bool usort(&array $array_arg, string $cmp_function)
 * Description: Sort an array by values using a user-defined comparison function
 * Source code: ext/standard/array.c
 */

/*
 * Pass an array of objects which are either:
 * 1. Empty
 * 2. Static
 * 2. Inherited
 * to test behaviour of usort()
 */

function cmp_function($value1, $value2)
:mixed{
  if($value1 == $value2) {
    return 0;
  }
  else if($value1 > $value2) {
    return 1;
  }
  else
    return -1;
}

// Class without any member
class EmptyClass
{
}

// Class with static member
class StaticClass
{
  public static $static_value;
  public function __construct($value) {
    StaticClass::$static_value = $value;
  }
}

// Abstract class
abstract class AbstractClass
{
  public $pub_value;
  public abstract function abstractMethod():mixed;
}

// Child class extending abstract class
class ChildClass extends AbstractClass
{
  public $child_value = 100;
  public function abstractMethod() :mixed{
    $pub_value = 5;
  }
  public function __construct($value) {
    $this->child_value = $value;
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing usort() : object functionality ***\n";

// Testing uasort with StaticClass objects as elements of 'array_arg'
echo "-- Testing usort() with StaticClass objects --\n";
$array_arg = dict[
  0 => new StaticClass(20),
  1 => new StaticClass(50),
  2 => new StaticClass(15),
  3 => new StaticClass(70),
];
var_dump( usort(inout $array_arg, cmp_function<>) );
var_dump($array_arg);

// Testing uasort with EmptyClass objects as elements of 'array_arg'
echo "-- Testing usort() with EmptyClass objects --\n";
$array_arg = dict[
  0 => new EmptyClass(),
  1 => new EmptyClass(),
  2 => new EmptyClass(),
  3 => new EmptyClass(),
];
var_dump( usort(inout $array_arg, cmp_function<>) );
var_dump($array_arg);

// Testing uasort with ChildClass objects as elements of 'array_arg'
echo "-- Testing usort() with ChildClass objects --\n";
$array_arg = dict[
  0 => new ChildClass(20),
  1 => new ChildClass(500),
  2 => new ChildClass(15),
  3 => new ChildClass(700),
];
var_dump( usort(inout $array_arg, cmp_function<>) );
var_dump($array_arg);
echo "===DONE===\n";
}
