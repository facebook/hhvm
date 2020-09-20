<?hh
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped
 * Source code: ext/standard/array.c
*/

/*
* Trying different invalid values for 'input' array argument
*/
// class definition for object data
class MyClass
{
   public function __toString()
   {
     return 'object';
   }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_flip() : different invalid values in 'input' array argument ***\n";

$obj = new MyClass();

// resource data
$fp = fopen(__FILE__, 'r');

$input = darray[
  // float values
  'float_value1' => 1.2,
  'float_value2' => 0.5,
  'flaot_value3' => 3.4E3,
  'flaot_value4' => 5.6E-6,

  // bool values
  'bool_value1' => true,
  'bool_value2' => false,
  'bool_value3' => TRUE,
  'bool_value4' => FALSE,

  // null values
  'null_value1' => null,

  // array value
  'array_value' => varray[1],

  // object value
  'obj_value' => $obj,

  // resource value
  'resource_value' => $fp,
];

var_dump( array_flip($input) );

// closing resource
fclose($fp);

echo "Done";
}
