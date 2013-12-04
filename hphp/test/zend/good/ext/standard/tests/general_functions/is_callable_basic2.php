<?php
ini_set('error_reporting ',  E_ALL & ~E_NOTICE | E_STRICT);

ini_set('precision', 14);

/* Prototype: bool is_callable ( mixed $var [, bool $syntax_only [, string &$callable_name]] );
   Description: Verify that the contents of a variable can be called as a function
                In case of objects, $var = array($SomeObject, 'MethodName')
*/

/* Prototype: void check_iscallable_objects( $methods );
   Description: use is_callable() on given $method to check if the array 
                contains a valid method name;
                returns true if valid function name, false otherwise
*/
function check_iscallable_objects( $methods ) {
  global $loop_counter;
  $counter = 1;
  foreach($methods as $method) {
    echo "-- Innerloop iteration $counter of Outerloop iteration $loop_counter --\n";
    var_dump( is_callable($method) );
    var_dump( is_callable($method, true) );
    var_dump( is_callable($method, false) );
    var_dump( is_callable($method, true, $callable_name) );
    echo $callable_name, "\n";
    var_dump( is_callable($method, false, $callable_name) );
    echo $callable_name, "\n";
    $counter++;
  }
}

echo "\n*** Testing is_callable() on objects ***\n";
class object_class
{
  public $value = 100;
  
  /* static method */
  static public function foo() {
  }
  
  public function foo1() {
  }
  /* function name with mixed string and integer */
  public function x123() {
  }
  /* function name as NULL */
  public function null() {
  }
  /* function name having boolean value */
  public function TRUE() {
  }  

  protected function foo2() {
  }
  private function foo3() {
  }
}
/* class with no member */
class no_member_class {
 // no members
}
/* class with member as object of other class */
class contains_object_class
{
   public    $class_object1;
   var       $no_member_class_object;

   public function func() {
     echo "func() is called \n";
   }

   function contains_object_class () {
     $this->class_object1 = new object_class();
     $this->no_member_class_object = new no_member_class();
   }
}
/* objects of different classes */
$obj = new contains_object_class;
$temp_class_obj = new object_class();

/* object which is unset */
$unset_obj = new object_class();
unset($unset_obj);

/* check is_callable() on static method */
echo "\n** Testing behavior of is_callable() on static methods **\n";
var_dump( is_callable('object_class::foo()', true) );   //expected: true
var_dump( is_callable('object_class::foo()') );    //expected: false

echo "\n** Testing normal operations of is_callable() on objects **\n";
$objects = array (
  new object_class,
  new no_member_class,
  new contains_object_class,
  $obj,
  $obj->class_object1,
  $obj->no_member_class_object,
  $temp_class_obj,
  @$unset_obj
);

/* loop to check whether given object/string has valid given method name
 *  expected: true if valid callback
 *            false otherwise
 */
$loop_counter = 1;
foreach($objects as $object) {
  echo "--- Outerloop iteration $loop_counter ---\n";
  $methods = array (
    array( $object, 'foo1' ),
    array( $object, 'foo2' ),
    array( $object, 'foo3' ),
    array( $object, 'x123' ),
    array( $object, 'null' ),
    array( $object, 'TRUE' ),
    array( $object, '123' ),
    array( @$temp_class_obj->value, 100 ),
    array( $object, 'func' ),
    array( 'object_class', 'foo1' ),
  );
  /* use check_iscallable_objects() to check whether given object/string
     has valid method name */
  check_iscallable_objects($methods);
  $loop_counter++;
}

?>
===DONE===