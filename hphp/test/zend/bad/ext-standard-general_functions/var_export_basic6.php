<?php
/* Prototype  : mixed var_export(mixed var [, bool return])
 * Description: Outputs or returns a string representation of a variable 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

echo "*** Testing var_export() with valid objects ***\n";

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
interface iValue
{
   public function setVal ($name, $val); 
   public function dumpVal ();
}
// implement the interface
class Value implements iValue
{
  private $vars = array ();
  
  public function setVal ( $name, $val ) {
    $this->vars[$name] = $val;
  }
  
  public function dumpVal () {
    var_export ( $vars );
  }
}

// a gereral class 
class myClass 
{
  var $foo_object;
  public $public_var;
  public $public_var1;
  private $private_var;
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
                  "new stdclass" => new stdclass,
                  "new foo" => new foo,
                  "new concreteClass" => new concreteClass,
                  "new Value" => new Value,
                  "new myClass" => new myClass,
                  "myClass_object" => $myClass_object,
                  "myClass_object->foo_object" => $myClass_object->foo_object,
                  "myClass_object->public_var1" => $myClass_object->public_var1,
                  "foo_object" => $foo_object,
                  "Value_object" => $Value_object,
                  "concreteClass_object" => $concreteClass_object
                 ); 
/* Loop to check for above objects with var_export() */
echo "\n*** Output for objects ***\n";
foreach($valid_objects as $key => $obj) {
	echo "\n-- Iteration: $key --\n";
	var_export( $obj );
	echo "\n";
	var_export( $obj, FALSE);
	echo "\n";
	var_dump( var_export( $obj, TRUE) );
	echo "\n";
}
?>
===DONE===