<?php
/* Prototype: bool is_object ( mixed $var );
 * Description: Finds whether the given variable is an object  
 */

echo "*** Testing is_object() with valid objects ***\n";

// class with no members
class foo
{
// no members 
}

// abstract class
abstract class abstractClass
{
  abstract protected function getClassName();
  public function printClassName () {
    echo $this->getClassName() . "\n";
  }
}

// implement abstract class
class concreteClass extends abstractClass
{
  protected function getClassName() {
    return "concreteClass";
  }
}

// interface class 
interface IValue
{
   public function setVal ($name, $val); 
   public function dumpVal ();
}

// implement the interface
class Value implements IValue
{
  private $vars = array ();
  
  public function setVal ( $name, $val ) {
    $this->vars[$name] = $val;
  }
  
  public function dumpVal () {
    var_dump ( $vars );
  }
}

// a gereral class 
class myClass 
{
  var       $foo_object;
  public    $public_var;
  public    $public_var1;
  private   $private_var;
  protected $protected_var;

  function myClass ( ) {
    $this->foo_object = new foo();
    $this->public_var = 10;
    $this->public_var1 = new foo();
    $this->private_var = new foo();
    $this->proected_var = new foo();
  }  
}

// create a object of each class defined above
$myClass_object = new myClass();
$foo_object = new foo();
$Value_object = new Value();
$concreteClass_object = new concreteClass();

$valid_objects = array(
  new stdclass,
  new foo,
  new concreteClass,
  new Value,
  new myClass,
  $myClass_object,
  $myClass_object->foo_object,
  $myClass_object->public_var1,
  $foo_object,
  $Value_object,
  $concreteClass_object
); 
                  
/* loop to check that is_object() recognizes different 
   objects, expected output: bool(true) */
$loop_counter = 1;
foreach ($valid_objects as $object ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_object($object) );
}

echo "\n*** Testing is_object() on non object types ***\n";

// get a resource type variable
$fp = fopen (__FILE__, "r");
$dfp = opendir ( dirname(__FILE__) );

// unset object 
$unset_object = new foo();
unset ($unset_object);

// other types in a array 
$not_objects = array (
  0,
  -1,
  0.1,
  -10.0000000000000000005,
  10.5e+5,
  0xFF,
  0123,
  $fp,  // resource
  $dfp,
  array(),
  array("string"),
  "0",
  "1",
  "",
  true,
  NULL,
  null,
  @$unset_object, // unset object
  @$undefined_var, // undefined variable
);
/* loop through the $not_objects to see working of 
   is_object() on non object types, expected output: bool(false) */
$loop_counter = 1;
foreach ($not_objects as $type ) {
  echo "-- Iteration $loop_counter --\n"; $loop_counter++;
  var_dump( is_object($type) );
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_dump( is_object() );

//arguments more than expected 
var_dump( is_object($myClass_object, $myClass_object) );
 
echo "Done\n";

// close the resources used
fclose($fp);
closedir($dfp);

?>