<?hh
/* Prototype  : proto bool is_subclass_of(object object, string class_name)
 * Description: Returns true if the object has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */
// Note: basic use cases in Zend/tests/is_a.phpt

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function is_subclass_of_variation_001(): void {
set_error_handler(test_error_handler<>);

echo "*** Testing is_subclass_of() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$class_name = 'string_val';


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
      'String',


];

// loop through each element of the array for object

foreach($values as $value) {
      echo "\nArg value ".(string)$value." \n";
      var_dump( is_subclass_of($value, $class_name) );
};

echo "Done";
}
