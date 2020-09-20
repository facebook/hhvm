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
  {
    return "object";
  }
}

<<__EntryPoint>> function main(): void {
echo "*** Testing str_split() : unexpected values for 'split_length' ***\n";

// Initialise function arguments
$str = 'variation2:split_length';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__, 'r');

//different values for 'split_length'
$values = varray[

  // float data
  10.5,
  -10.5,
  10.6E10,
  10.6E-10,
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
  new MyClass(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,

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
