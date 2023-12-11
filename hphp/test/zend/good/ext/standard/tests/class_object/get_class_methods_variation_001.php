<?hh
/* Prototype  : proto array get_class_methods(mixed class)
 * Description: Returns an array of method names for class or class instance.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */


function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function get_class_methods_variation_001(): void {
set_error_handler(test_error_handler<>);
echo "*** Testing get_class_methods() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)


//array of values to iterate over
$values = vec[

      // int data
      0,
      1,
      12345,
      -2345,

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // string data
      "string",
      'string',

      // object data
      new stdClass(),


];

// loop through each element of the array for class

foreach($values as $value) {
      echo "\nArg value ".(string)$value." \n";
      var_dump( get_class_methods($value) );
};

echo "Done";
}
