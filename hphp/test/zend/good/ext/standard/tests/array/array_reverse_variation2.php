<?hh
/* Prototype  : array array_reverse(array $array [, bool $preserve_keys])
 * Description: Return input as a new array with the order of the entries reversed
 * Source code: ext/standard/array.c
*/

/*
 * testing the functionality of array_reverse() by giving unexpected values for $preserve_keys argument
*/

//get a class
class classA
{
  public function __toString(){
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_reverse() : usage variations ***\n";

// Initialise the array
$array = darray["a" => "green", 0 => "red", 1 => "blue", 2 => "red", 3 => "orange", 4 => "pink"];

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//get a resource variable
$fp = fopen(__FILE__, "r");

//array of values to iterate over
$preserve_keys = varray [

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       10.5e10,
       10.6E-10,
       .5,

       // array data
/*10*/ varray[],
       varray[0],
       varray[1],
       varray[1, 2],
       darray['color' => 'red', 'item' => 'pen'],

       // null data
/*15*/ NULL,
       null,

       // boolean data
       true,
       false,
       TRUE,
       FALSE,

       // empty data
/*21*/
       "",
       '',

       // object data
       new classA(),

       // undefined data
       @$undefined_var,

       // unset data
       @$unset_var,

       // resource variable
/*26*/ $fp

];

// loop through each element of the array $preserve_keys to check the behavior of array_reverse()
$iterator = 1;
foreach($preserve_keys as $preserve_key) {
  echo "-- Iteration $iterator --\n";
  try { var_dump( array_reverse($array, $preserve_key) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  $iterator++;
};

// close the file resouce used
fclose($fp);

echo "Done";
}
