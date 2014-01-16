<?php
ini_set('precision', 14);

/* Prototype: void var_dump ( mixed $expression [, mixed $expression [, $...]] );
   Description: Displays structured information about one or more expressions that includes its type and value.
*/

/* Prototype: void check_vardump( $variables );
   Description: use var_dump() to display the variables */
function check_vardump( $variables ) {
  $counter = 1;
  foreach( $variables as $variable ) {
    echo "-- Iteration $counter --\n";
    var_dump($variable);
    $counter++;
  }
}
  
echo "\n*** Testing var_dump() on integer variables ***\n";
$integers = array ( 
  0,  // zero as argument
  000000123,  //octal value of 83
  123000000,
  -00000123,  //octal value of 83
  -12300000,
  range(1,10),  // positive values
  range(-1,-10),  // negative values
  +2147483647,  // max positive integer
  +2147483648,  // max positive integer + 1
  -2147483648,  // min range of integer
  -2147483647,  // min range of integer + 1
  0x7FFFFFFF,  // max positive hexadecimal integer
  -0x80000000,  // min range of hexadecimal integer
  017777777777,  // max posotive octal integer
  -020000000000  // min range of octal integer
);		    
/* calling check_vardump() to display contents of integer variables 
   using var_dump() */
check_vardump($integers);

echo "\n*** Testing var_dump() on float variables ***\n";
$floats = array (
  -0.0,
  +0.0,
  1.234,
  -1.234,
  -2.000000,
  000002.00,
  -.5,
  .567,
  -.6700000e-3,
  -.6700000E+3,
  .6700000E+3,
  .6700000e+3,
  -4.10003e-3,
  -4.10003E+3,
  4.100003e-3,
  4.100003E+3,
  1e5,
  -1e5,
  1e-5,
  -1e-5,
  1e+5,
  -1e+5,
  1E5,
  -1E5,
  1E+5,
  -1E+5,
  1E-5,
  -1E-5,
  -0x80000001,  // float value, beyond max negative int
  0x80000001,  // float value, beyond max positive int
  020000000001,  // float value, beyond max positive int
  -020000000001  // float value, beyond max negative int 
);
/* calling check_vardump() to display contents of float variables 
   using var_dump() */
check_vardump($floats);

echo "\n*** Testing var_dump() on string variables ***\n";
$strings = array (
  "",
  '',
  " ",
  ' ',
  "0",
  "\0",
  '\0',
  "\t",
  '\t',
  "PHP",
  'PHP',
  "abcd\x0n1234\x0005678\x0000efgh\xijkl",  // strings with hexadecimal NULL
  "abcd\0efgh\0ijkl\x00mnop\x000qrst\00uvwx\0000yz",  // strings with octal NULL
  "1234\t\n5678\n\t9100\rabcda"  // strings with escape characters
);
/* calling check_vardump() to display contents of strings 
   using var_dump() */
check_vardump($strings);

echo "\n*** Testing var_dump() on boolean variables ***\n";
$booleans = array (
  TRUE,
  FALSE,
  true,
  false
);	  
/* calling check_vardump() to display boolean variables
   using var_dump() */
check_vardump($booleans);

echo "\n*** Testing var_dump() on array variables ***\n";
$arrays = array (
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
  array('string', 'test'),
);
/* calling check_vardump() to display contents of an array
   using var_dump() */
check_vardump($arrays);

echo "\n*** Testing var_dump() on object variables ***\n";
class object_class
{
  var       $value;
  public    $public_var1 = 10;
  private   $private_var1 = 20;
  private   $private_var2;
  protected $protected_var1 = "string_1";
  protected $protected_var2;

  function object_class ( ) {
    $this->value = 50;
    $this->public_var2 = 11;
    $this->private_var2 = 21;
    $this->protected_var2 = "string_2";
  }

  public function foo1() {
    echo "foo1() is called\n";
  }
  protected function foo2() {
    echo "foo2() is called\n";
  }
  private function foo3() {
    echo "foo3() is called\n";
  }
}
/* class with no member */
class no_member_class {
 // no members
}

/* class with member as object of other class */
class contains_object_class
{
   var       $p = 30;
   var       $class_object1;
   public    $class_object2;
   private   $class_object3;
   protected $class_object4;
   var       $no_member_class_object;

   public function func() {
     echo "func() is called \n";
   }

   function contains_object_class () {
     $this->class_object1 = new object_class();
     $this->class_object2 = new object_class();
     $this->class_object3 = $this->class_object1;
     $this->class_object4 = $this->class_object2;
     $this->no_member_class_object = new no_member_class();
     $this->class_object5 = $this;   //recursive reference
   }
}

/* objects of different classes */
$obj = new contains_object_class;
$temp_class_obj = new object_class();

/* object which is unset */
$unset_obj = new object_class();
unset($unset_obj);

$objects = array (
  new object_class,
  new no_member_class,
  new contains_object_class,
  $obj,
  $obj->class_object1,
  $obj->class_object2,
  $obj->no_member_class_object,
  $temp_class_obj,
  @$unset_obj
);
/* calling check_vardump() to display contents of the objects
   using var_dump() */
check_vardump($objects);

echo "\n** Testing var_dump() on objects having circular reference **\n";
$recursion_obj1 = new object_class();
$recursion_obj2 = new object_class();
$recursion_obj1->obj = &$recursion_obj2;  //circular reference
$recursion_obj2->obj = &$recursion_obj1;  //circular reference
var_dump($recursion_obj2);

echo "\n*** Testing var_dump() on resources ***\n";
/* file type resource */
$file_handle = fopen(__FILE__, "r"); 

/* directory type resource */
$dir_handle = opendir( dirname(__FILE__) );

$resources = array (
  $file_handle,
  $dir_handle
);
/* calling check_vardump() to display the resource content type
   using var_dump() */
check_vardump($resources);

echo "\n*** Testing var_dump() on different combinations of scalar 
            and non-scalar variables ***\n";
/* a variable which is unset */
$unset_var = 10.5;
unset($unset_var);

/* unset file type resource */
unset($file_handle);

$variations = array (
  array( 123, -1.2345, "a" ),
  array( "d", array(1, 3, 5), true, null),
  array( new no_member_class, array(), false, 0 ),
  array( -0.00, "Where am I?", array(7,8,9), TRUE, 'A', 987654321 ),
  array( @$unset_var, 2.E+10, 100-20.9, 000004.599998 ),  //unusual data
  array( "array(1,2,3,4)1.0000002TRUE", @$file_handle, 111333.00+45e5, '/00\7') 
);
/* calling check_vardump() to display combinations of scalar and
   non-scalar variables using var_dump() */
check_vardump($variations);

echo "\n*** Testing var_dump() on miscelleneous input arguments ***\n";
$misc_values = array (
  @$unset_var,
  NULL,  // NULL argument
  @$undef_variable,  //undefined variable
  null
);
/* calling check_vardump() to display miscelleneous data using var_dump() */
check_vardump($misc_values);

echo "\n*** Testing var_dump() on multiple arguments ***\n";
var_dump( $integers, $floats, $strings, $arrays, $booleans, $resources, 
          $objects, $misc_values, $variations );

/* checking var_dump() on functions */
echo "\n*** Testing var_dump() on anonymous functions ***\n";
$newfunc = create_function('$a,$b', 'return "$a * $b = " . ($a * $b);');
echo "New anonymous function: $newfunc\n";
var_dump( $newfunc(2, 3) );
/* creating anonymous function dynamically */
var_dump( create_function('$a', 'return "$a * $a = " . ($a * $b);') );

echo "\n*** Testing error conditions ***\n";
//passing zero argument
var_dump();

/* closing resource handle used */
closedir($dir_handle); 

echo "Done\n";
?>