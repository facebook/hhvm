<?hh
/* Prototype  : string chunk_split(string $str [, int $chunklen [, string $ending]])
 * Description: Returns split line
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/
//Class to get object variable
class MyClass
{
   public function __toString() :mixed{
     return "object";
   }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing chunk_split() : with unexpected values for 'chunklen' argument ***\n";

// Initialise function arguments
$str = 'This is chuklen variation';
$ending = '*';


//get resource variable
$fp = fopen(__FILE__, 'r');

//array of values to iterate over
$values = vec[

  // float data
  10.5,
  -10.5,
  (float) PHP_INT_MAX + 1,
  (float) -PHP_INT_MAX - 1,
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



  // resource variable
  $fp
];

// loop through each element of the values for 'chunklen'
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  try { var_dump( chunk_split($str, $values[$count], $ending) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

//closing resource
fclose($fp);

echo "===DONE===\n";
}
