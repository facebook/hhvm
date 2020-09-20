<?hh
/* Prototype  : array array_chunk(array $array, int $size [, bool $preserve_keys])
 * Description: Split array into chunks
              : Chunks an array into size  large chunks
 * Source code: ext/standard/array.c
*/

/*
* Testing array_chunk() function with unexpected values for 'size' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_chunk() : usage variations ***\n";

// input array
$input = varray[1, 2];

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = varray [

        // float data
/*1*/   10.5,
        -10.5,
        10.5e10,
        10.6E-10,
        .5,

        // array data
/*6*/   varray[],
        varray[0],
        varray[1],
        varray[1, 2],
        darray['color' => 'red', 'item' => 'pen'],

        // null data
/*11*/  NULL,
        null,

        // boolean data
/*13*/  true,
        false,
        TRUE,
        FALSE,

        // empty data
/*17*/  "",
        '',

        // string data
/*19*/  "string",
        'string',

        // object data
/*21*/  new stdclass(),

];

// loop through each element of the array for size
$count = 1;
foreach($values as $value){
  echo "\n-- Iteration $count --\n";
  try { var_dump( array_chunk($input, $value) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( array_chunk($input, $value, true) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  try { var_dump( array_chunk($input, $value, false) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $count++;
}

echo "Done";
}
