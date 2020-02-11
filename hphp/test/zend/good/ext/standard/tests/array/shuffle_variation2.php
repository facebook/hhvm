<?hh
/* Prototype  : bool shuffle(&array $array_arg)
 * Description: Randomly shuffle the contents of an array
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of shuffle() function when multi-dimensional array is
* passed to 'array_arg' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing shuffle() : with multi-dimensional array ***\n";

// initialise the multi-dimensional array
$array_arg = varray[
  varray[1, 2, 3],
  varray[4, 5, 6],
  varray[7, 8, 9],
  varray[10000, 20000000, 30000000],
  varray[0, 0, 0],
  varray[012, 023, 034],
  varray[0x1, 0x0, 0xa]

];

// calling shuffle() function with multi-dimensional array
var_dump( shuffle(inout $array_arg) );
echo "\nThe output array is:\n";
var_dump( $array_arg );


echo "Done";
}
