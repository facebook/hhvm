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
echo "*** Testing str_split() : unexpected values for 'str' ***\n";

// Initialise function arguments
$split_length = 3;

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//resource variable
$fp = fopen(__FILE__, 'r');

//different values for 'str' argument
$values = varray[

  // int data
  0,
  1,
  12345,
  -2345,

  // float data
  10.5,
  -10.5,
  10.5e10,
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

  // object data
  new MyClass(),

  // undefined data
  @$undefined_var,

  // unset data
  @$unset_var,

  //resource data
  $fp
];

// loop through each element of $values for 'str' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  try { var_dump( str_split($values[$count], $split_length) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}

//closing resource
fclose($fp);

echo "Done";
}
