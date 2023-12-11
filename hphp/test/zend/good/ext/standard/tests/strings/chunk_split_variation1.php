<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line %d%d
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/
//class for object variable
class MyClass
{
  public function __toString()
:mixed  {
    return "object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : with unexpected values for 'str' argument ***\n";

// Initialising variables
$chunklen = 2;
$ending = ' ';


//resource  variable
$fp = fopen(__FILE__, 'r');

//different values for 'str'
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

  // string data
  "string",
  'string',

  // object data
  new MyClass(),



  // resource data
  $fp
];

// loop through each element of the array for 'str'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  try { var_dump( chunk_split($values[$count], $chunklen, $ending) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
};

echo "Done";

// close the resource
fclose($fp);
}
