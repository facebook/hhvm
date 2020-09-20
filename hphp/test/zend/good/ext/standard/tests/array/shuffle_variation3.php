<?hh
/* Prototype  : bool shuffle(&array $array_arg)
 * Description: Randomly shuffle the contents of an array
 * Source code: ext/standard/array.c
*/

/*
* Test behaviour of shuffle() function when arrays having different
* types of values, are passed to 'array_arg' argument
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing shuffle() : arrays with diff types of values ***\n";

// initialise different arrays
$array_arg = varray[
       // array with positive int values
/*1*/  varray[0, 1, 2, 2147483647 ],

       // array with negative int values
       varray[-1, -2, -2147483647 ],

       // array with positive float values
/*3*/  varray[0.23, 1.34, 0e2, 200e-2, 30e2, 10e0, 2147473648.90],

       // array with negative float values
       varray[-0.23, -1.34, -200e-2, -30e2, -10e0, -2147473649.80],

       // array with single quoted and double quoted strings
/*5*/  varray['one', "123numbers", 'hello\tworld', "hello world\0", '12.34floatnum'],

       // array with bool values
       varray[true, TRUE, FALSE, false],

       // array with positive hexa values
/*7*/  varray[0x123, 0xabc, 0xABC, 0xac, 0xAb1, 0x9fa],

       // array with negative hexa values
       varray[-0x123, -0xabc, -0xABC, -0xAb1, -0x9fa],

       // array with positive octal values
/*9*/  varray[0123, 02348, 034, 00],

       // array with negative octal values
/*10*/ varray[-0123, -02348, -034],

];

// looping to test shuffle() with each sub-array in the $array_arg array
echo "\n*** Testing shuffle() with arrays having different types of values ***\n";
$counter = 1;
foreach($array_arg as $arr) {
  echo "\n-- Iteration $counter --\n";
  var_dump( shuffle(inout $arr) );
  echo "\nThe output array is:\n";
  var_dump( $arr );
  $counter++;
}

echo "Done";
}
