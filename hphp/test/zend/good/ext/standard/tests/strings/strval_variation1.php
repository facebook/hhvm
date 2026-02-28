<?hh
/* Prototype  : string strval  ( mixed $var  )
 * Description: Get the string value of a variable.
 * Source code: ext/standard/string.c
 */

class MyClass
{
  function __toString() :mixed{
    return "MyClass";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing strval() : usage variations ***\n";

error_reporting(E_ALL ^ E_NOTICE);


//getting the resource
$file_handle = fopen(__FILE__, "r");

//array of values to iterate over
$values = vec[
          //Decimal values
/*1*/      0,
          1,
          12345,
          -12345,

          //Octal values
/*5*/      02,
          010,
          030071,
          -030071,

          //Hexadecimal values
/*9*/      0x0,
          0x1,
          0xABCD,
          -0xABCD,

          // float data
/*13*/    100.5,
          -100.5,
          100.1234567e10,
          100.7654321E-10,
          .5,

          // array data
/*18*/    'Array',
          'Array',

          // null data
/*20*/    NULL,
          null,

          // boolean data
/*22*/    true,
          false,
          TRUE,
          FALSE,

          // empty data
/*26*/    "",
          '',

          // object data
/*28*/    new MyClass(),

          // resource
/*29*/    $file_handle,


];

// loop through each element of the array for strval
$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( strval($value) );
      $iterator++;
};
echo "===DONE===\n";
}
