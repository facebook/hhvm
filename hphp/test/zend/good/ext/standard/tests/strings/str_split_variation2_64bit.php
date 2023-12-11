<?hh
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is
                specified, break the string down into chunks each
                split_length characters long.
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/
//defining class for object variable
class MyClass
{
  public function __toString()
:mixed  {
    return "object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing str_split() : unexpected values for 'split_length' ***\n";

// Initialise function arguments
$str = 'variation2:split_length';


//resource variable
$fp = fopen(__FILE__, 'r');

//different values for 'split_length'
$values = vec[

  // float data
  10.5,
  -10.5,
  10.6E10,
  10.6E-10,
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



  //resource data
  $fp
];

// loop through each element of $values for 'split_length'
for($count = 0; $count < count($values); $count++) {
  echo "--Iteration ".($count+1)." --\n";
  try { var_dump( str_split($str, $values[$count]) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

//closing resource
fclose($fp);

echo "Done";
}
