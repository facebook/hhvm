<?hh

<<__EntryPoint>>
function entrypoint_utf8_encode_variation1(): void {
  /* Prototype  : proto string utf8_encode(string data)
   * Description: Encodes an ISO-8859-1 string to UTF-8
   * Source code: ext/xml/xml.c
   * Alias to functions:
   */

  echo "*** Testing utf8_encode() : usage variations ***\n";
  error_reporting(E_ALL & ~E_NOTICE);

  // Initialise function arguments not being substituted (if any)


  //array of values to iterate over
  $values = vec[
        // empty data
        "",
        '',
  ];

  // loop through each element of the array for data

  foreach($values as $value) {
        echo @"\nArg value $value \n";
        try { var_dump( utf8_encode($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  }

  echo "Done";
}
