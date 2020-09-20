<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/
//Class to get object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : unexpected values for 'ending' ***\n";

// Initializing variables
$str = 'This is simple string.';
$chunklen = 4.9;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__,'r');

//different values for 'ending'
$values = varray[

  // int data
  0,
  1,
  12345,
  -2345,

  // float data
  10.5,
  -10.5,
  10.123456e10,
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

  // object data
  new MyClass(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,

  // resource data
  $fp
];

// loop through each element of values for 'ending'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  try { var_dump( chunk_split($str, $chunklen, $values[$count]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

echo "Done";

//closing resource
fclose($fp);
}
