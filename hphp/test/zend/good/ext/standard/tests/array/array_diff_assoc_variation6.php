<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments but do additional checks whether the keys are equal
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc behaves
 * 1. When comparing an array that has similar elements
 *    but has been created in a different order
 * 2. When doing a strict comparison of string representation
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff_assoc() : usage variations ***\n";

$array = darray[0 => 'zero',
                1 => 1,
                'two' => 2.00000000000001];

$inputs = varray [

//default keys => string values
/*1*/    varray['2.00000000000001', '1', 'zero', 'a'],

//numeric keys => string values
/*2*/    darray[2 => '2.00000000000001',
              1 => '1',
              0 => 'zero',
              3 => 'a'],

//string keys => string values
/*3*/    darray['2' => '2.00000000000001',
              '1' => '1',
              '0' => 'zero',
              '3' => 'a'] ,

//default keys => numeric values
/*4*/    varray[2, 1, 0],

//numeric keys => numeric values
/*5*/    darray[2 => 2,
              1 => 1,
              0 => 0],

//string keys => numeric values
/*6*/    darray['two' => 2,
              '1' => 1,
              '0' => 0],

//defualt keys => float values
/*7*/    varray[2.00000000000001, 1.00, 0.01E-9],

//numeric keys => float values
/*8*/    darray[2 => 2.00000000000001,
              1 =>  1.00,
              0 => 0.01E-9],

//string keys => float values
/*9*/    darray ['two' => 2.00000000000001,
               '1' => 1.00,
               '0' =>0.01E-9]
];

// loop through each element of $inputs to check the behavior of array_diff_assoc
$iterator = 1;
foreach($inputs as $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump(array_diff_assoc($array, $input));
  var_dump(array_diff_assoc($input, $array));
  $iterator++;
};
echo "Done";
}
