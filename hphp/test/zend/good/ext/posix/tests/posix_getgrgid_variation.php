<?hh
/* Prototype  : proto array posix_getgrgid(long gid)
 * Description: Group database access (POSIX.1, 9.2.1)
 * Source code: ext/posix/posix.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_getgrgid() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = varray[

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      varray[],
      varray[0],
      varray[1],
      varray[1, 2],
      darray['color' => 'red', 'item' => 'pen'],

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

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,

      // object data
      new stdclass(),
];

// loop through each element of the array for gid

foreach($values as $value) {
  echo "\nArg value $value\n";
  try { $result = posix_getgrgid($value);
  if ((is_array($result) && (count($result) == 4))
      ||
      ($result === false)) {
      echo "valid output\n";
  } else {
      var_dump($result);
  }
  } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
