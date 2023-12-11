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
:mixed  {
    return "object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : unexpected values for 'ending' ***\n";

// Initializing variables
$str = 'This is simple string.';
$chunklen = 4.9;


//resource variable
$fp = fopen(__FILE__,'r');

//different values for 'ending'
$values = vec[

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
  new MyClass(),



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
