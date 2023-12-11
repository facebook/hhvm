<?hh
/* Prototype  : array array_map  ( callback $callback  , array $arr1  [, array $...  ] )
 * Description: Applies the callback to the elements of the given arrays
 * Source code: ext/standard/array.c
 */

/*
 * Test array_map() by passing different scalar/nonscalar values in place of $callback
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : unexpected values for 'callback' argument ***\n";

$arr1 = vec[1, 2, 3];

// get a resource variable
$fp = fopen(__FILE__, "r");

// unexpected values to be passed to $input argument
$unexpected_callbacks = vec[

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // boolean data
/*10*/ true,
       false,
       TRUE,
       FALSE,

       // empty data
/*14*/ "",
       '',

       // array data
/*16*/ vec[],
       vec[1, 2],
       vec[1, vec[2]],

       // object data
/*19*/ new classA(),

       // resource variable
/*20*/ $fp
];

// loop through each element of $inputs to check the behavior of array_map
for($count = 0; $count < count($unexpected_callbacks); $count++) {
  echo "\n-- Iteration ".($count + 1)." --";
  var_dump( array_map($unexpected_callbacks[$count], $arr1));
};

fclose($fp);
echo "Done";
}
