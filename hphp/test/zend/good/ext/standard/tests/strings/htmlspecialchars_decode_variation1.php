<?hh
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters
 * Source code: ext/standard/html.c
*/

/*
 * testing htmlspecialchars_decode() with unexpected input values for $string argument
*/

//get a class
class classA
{
  function __toString() :mixed{
    return "ClassAObject";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing htmlspecialchars_decode() : usage variations ***\n";

//get a resource variable
$file_handle=fopen(__FILE__, "r");


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

      // object data
      new classA(),



      //resource
      $file_handle
];

// loop through each element of the array for string
$iterator = 1;
foreach($values as $value) {
      echo "-- Iterator $iterator --\n";
      try { var_dump( htmlspecialchars_decode($value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
      $iterator++;
};

// close the file resource used
fclose($file_handle);

echo "===DONE===\n";
}
