<?hh
/* Prototype  : proto bool posix_kill(int pid, int sig)
 * Description: Send a signal to a process (POSIX.1, 3.3.2)
 * Source code: ext/posix/posix.c
 * Alias to functions:
 */
<<__EntryPoint>>
function main(): void {
  echo "*** Testing posix_kill() : usage variations ***\n";

  // Initialise function arguments not being substituted (if any)
  $pid = -999;


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

  // loop through each element of the array for sig

  foreach ($values as $value) {
    $text = HH\is_any_array($value) ? 'Array' : $value; echo "\nArg value ".(string)$text."\n";
    try {
      var_dump(posix_kill($pid, $value));
    } catch (Exception $e) {
      echo "\n".
        'Warning: '.
        $e->getMessage().
        ' in '.
        __FILE__.
        ' on line '.
        __LINE__.
        "\n";
    }
  }
  ;

  echo "Done";
}
