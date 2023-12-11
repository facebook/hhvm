<?hh
/* Prototype  : proto string posix_strerror(int errno)
 * Description: Retrieve the system error message associated with the given errno.
 * Source code: ext/posix/posix.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_strerror() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)


//array of values to iterate over
$values = vec[

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      vec[],
      vec[0],
      vec[1],
      vec[1, 2],
      dict['color' => 'red', 'item' => 'pen'],

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

// loop through each element of the array for errno

foreach($values as $value) {
      $text = HH\is_any_array($value) ? 'Array' : $value; echo "\nArg value ".(string)$text."\n";
      try { echo gettype( posix_strerror($value) )."\n"; } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
