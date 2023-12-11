<?hh


function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) :mixed{
	echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
<<__EntryPoint>>
function entrypoint_base64_encode_variation_001(): void {
  /* Prototype  : proto string base64_encode(string str)
   * Description: Encodes string using MIME base64 algorithm
   * Source code: ext/standard/base64.c
   * Alias to functions:
   */

  echo "*** Testing base64_encode() : usage variations ***\n";
  set_error_handler(test_error_handler<>);

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

        // object data
        new stdClass()
  ];

  // loop through each element of the array for str

  foreach($values as $value) {
        echo "\nArg value ".(string)$value."\n";
        try { var_dump( base64_encode($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }

  echo "Done";
}
