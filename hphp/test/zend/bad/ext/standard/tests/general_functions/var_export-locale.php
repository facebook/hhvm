<?php
ini_set('serialize_precision', 17);

setlocale(LC_ALL, "german", "de","de_DE","de_DE.ISO8859-1","de_DE.ISO_8859-1","de_DE.UTF-8");
/* Prototype: mixed var_export( mixed expression [, bool return]);
 * Description: Returns the variable representation when the return parameter is used and evaluates to TRUE. Otherwise, this function will return NULL.

*/

echo "*** Testing var_export() with integer values ***\n";
// different integer vlaues 
$valid_ints = array(
                '0',
                '1',
                '-1',
                '-2147483648', // max negative integer value
                '-2147483647', 
                2147483647,  // max positive integer value
                2147483640,
                0x123B,      // integer as hexadecimal
                '0x12ab',
                '0Xfff',
                '0XFA',
                -0x80000000, // max negative integer as hexadecimal
                '0x7fffffff',  // max postive integer as hexadecimal
                0x7FFFFFFF,  // max postive integer as hexadecimal
                '0123',        // integer as octal
                01912,       // should be quivalent to octal 1
                -020000000000, // max negative integer as octal
                017777777777,  // max positive integer as octal
               );
$counter = 1;
/* Loop to check for above integer values with var_export() */
echo "\n*** Output for integer values ***\n";
foreach($valid_ints as $int_value) {
echo "\nIteration ".$counter."\n";
var_export( $int_value );
echo "\n";
var_export( $int_value, FALSE);
echo "\n";
var_dump( var_export( $int_value, TRUE) );
echo "\n";
$counter++;
}

echo "*** Testing var_export() with valid boolean values ***\n";
// different valid  boolean vlaues 
$valid_bool = array(
		    1,
		    TRUE,
                true, 
                0,
		    FALSE,
		    false
               );
$counter = 1;
/* Loop to check for above boolean values with var_export() */
echo "\n*** Output for boolean values ***\n";
foreach($valid_bool as $bool_value) {
echo "\nIteration ".$counter."\n";
var_export( $bool_value );
echo "\n";
var_export( $bool_value, FALSE);
echo "\n";
var_dump( var_export( $bool_value, TRUE) );
echo "\n";
$counter++;
}

echo "*** Testing var_export() with valid float values ***\n";
// different valid  float vlaues 
$valid_floats = array(
  -2147483649, // float value
  2147483648,  // float value
  -0x80000001, // float value, beyond max negative int
  0x800000001, // float value, beyond max positive int
  020000000001, // float value, beyond max positive int
  -020000000001, // float value, beyond max negative int
  0.0,
  -0.1,
  10.0000000000000000005,
  10.5e+5,
  1e5,
  1e-5,
  1e+5,
  1E5,
  1E+5,
  1E-5,
  .5e+7,
  .6e-19,
  .05E+44,
  .0034E-30
);
$counter = 1;
/* Loop to check for above float values with var_export() */
echo "\n*** Output for float values ***\n";
foreach($valid_bool as $float_value) {
echo "\nIteration ".$counter."\n";
var_export( $float_value );
echo "\n";
var_export( $float_value, FALSE);
echo "\n";
var_dump( var_export( $float_value, TRUE) );
echo "\n";
$counter++;
}

echo "*** Testing var_export() with valid strings ***\n";
// different valid  string 
$valid_strings = array(
            "",
            " ",
            '',
            ' ',
            "string",
            'string',
            "NULL",
            'null',
            "FALSE",
            'false',
            "\x0b",
            "\0",
            '\0',
            '\060',
            "\070"
          );
$counter = 1;
/* Loop to check for above strings with var_export() */
echo "\n*** Output for strings ***\n";
foreach($valid_strings as $str) {
echo "\nIteration ".$counter."\n";
var_export( $str );
echo "\n";
var_export( $str, FALSE);
echo "\n";
var_dump( var_export( $str, TRUE) );
echo "\n";
$counter++;
}

echo "*** Testing var_export() with valid arrays ***\n";
// different valid  arrays 
$valid_arrays = array(
           array(),
           array(NULL),
           array(null),
           array(true),
           array(""),
           array(''),
           array(array(), array()),
           array(array(1, 2), array('a', 'b')),
           array(1 => 'One'),
           array("test" => "is_array"),
           array(0),
           array(-1),
           array(10.5, 5.6),
           array("string", "test"),
           array('string', 'test')
          );
$counter = 1;
/* Loop to check for above arrays with var_export() */
echo "\n*** Output for arrays ***\n";
foreach($valid_arrays as $arr) {
echo "\nIteration ".$counter."\n";
var_export( $arr );
echo "\n";
var_export( $arr, FALSE);
echo "\n";
var_dump( var_export( $arr, TRUE) );
echo "\n";
$counter++;
}

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
 $counter = 1;
/* Loop to check for above objects with var_export() */
echo "\n*** Output for objects ***\n";
foreach($valid_objects as $obj) {
echo "\nIteration ".$counter."\n";
var_export( $obj );
echo "\n";
var_export( $obj, FALSE);
echo "\n";
var_dump( var_export( $obj, TRUE) );
echo "\n";
$counter++;
}
                 
echo "*** Testing var_export() with valid null values ***\n";
// different valid  null vlaues 
$unset_var = array();
unset ($unset_var); // now a null
$null_var = NULL;

$valid_nulls = array(
                NULL,
                null,
                $null_var,
               );
 $counter = 1;
/* Loop to check for above null values with var_export() */
echo "\n*** Output for null values ***\n";
foreach($valid_nulls as $null_value) {
echo "\nIteration ".$counter."\n";
var_export( $null_value );
echo "\n";
var_export( $null_value, FALSE);
echo "\n";
var_dump( var_export( $null_value, true) );
echo "\n";
$counter++;
}

echo "\n*** Testing error conditions ***\n";
//Zero argument
var_export( var_export() );

//arguments more than expected 
var_export( var_export(TRUE, FALSE, TRUE) );
 
echo "\n\nDone";


?>