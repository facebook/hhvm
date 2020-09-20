<?hh
/* Prototype  : proto string urldecode(string str)
 * Description: Decodes URL-encoded string
 * Source code: ext/standard/url.c
 * Alias to functions:
 */

// NB: basic functionality tested in tests/strings/001.phpt

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
    echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>> function main(): void {
set_error_handler(fun('test_error_handler'));
echo "*** Testing urldecode() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = varray[

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
      new stdclass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
];

// loop through each element of the array for str

foreach($values as $value) {
      echo "\nArg value $value\n";
      try { var_dump( urldecode($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
