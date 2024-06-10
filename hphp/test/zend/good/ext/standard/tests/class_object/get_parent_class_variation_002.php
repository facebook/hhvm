<?hh
/* Prototype  : proto string get_parent_class([mixed object])
 * Description: Retrieves the parent class name for object or class or current scope.
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function get_parent_class_variation_002(): void {
set_error_handler(test_error_handler<>);
echo "*** Testing get_parent_class() : usage variations ***\n";


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

      // array data
      'Array',
      'Array',
      'Array',
      'Array',
      'Array',

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
      'String',

      // object data
      new stdClass(),


];

// loop through each element of the array for object

foreach($values as $value) {
      echo "\nArg value ".(string)$value." \n";
      var_dump( get_parent_class($value) );
};

echo "Done";
}
