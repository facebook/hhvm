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
  // empty data
  "",
  '',

  // string data
  "string",
  'string',
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
