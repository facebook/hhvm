<?hh
/* Prototype  : proto int fpassthru(resource fp)
 * Description: Output all remaining data from a file pointer
 * Source code: ext/standard/file.c
 * Alias to functions: gzpassthru
 */

class testClass {
   public function __toString() {
      return "testClass";
   }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing fpassthru() : usage variations ***\n";
error_reporting(E_ALL & ~E_NOTICE);

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

      // object data
      new testClass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
];

// loop through each element of the array for fp

foreach($values as $value) {
      echo @"\nArg value $value \n";
      try { var_dump( fpassthru($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";
}
