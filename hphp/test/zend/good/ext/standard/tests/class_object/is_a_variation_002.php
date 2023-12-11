<?hh
/* Prototype  : proto bool is_a(object object, string class_name)
 * Description: Returns true if the object is of this class or has this class as one of its parents
 * Source code: Zend/zend_builtin_functions.c
 * Alias to functions:
 */

class C {
    function __toString() :mixed{
        return "C Instance";
    }
}
<<__EntryPoint>> function is_a_variation_002(): void {
echo "*** Testing is_a() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$object = new stdClass();


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

      // object data
      new C,


];

// loop through each element of the array for class_name

foreach($values as $value) {
      echo @"\nArg value $value \n";
      try { var_dump( is_a($object, $value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
