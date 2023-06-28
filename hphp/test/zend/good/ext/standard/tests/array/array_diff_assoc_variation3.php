<?hh
/* Prototype  : array array_diff_assoc(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of arr1 that have values which are not present
 * in any of the others arguments but do additional checks whether the keys are equal
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff_assoc() compares indexed arrays containing different data types
 */

// get a class
class classA
{
  public function __toString() :mixed{
    return "Class A object";
  }
}
<<__EntryPoint>> function main(): void {
echo "\n*** Testing array_diff_assoc() : usage variations ***\n";

$array = varray[1, 2, 3];


// heredoc string
$heredoc = <<<EOT
hello world
EOT;

//array of different data types to be passed to $arr1 argument
$inputs = darray[

       // int data
/*1*/
'int' => varray[
       0,
       1,
       12345,
       -2345],

       // float data
/*2*/
'float' => varray[
       10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5],

       // null data
/*3*/
'null' => varray[
       NULL,
       null],

       // boolean data
/*4*/
'bool' => varray[
       true,
       false,
       TRUE,
       FALSE],

       // empty data
/*5*/
'empty' => varray[
       "",
       ''],

       // string data
/*6*/
'string' => varray[
       "string",
       'string',
       $heredoc],

       // binary data
/*7*/
'binary' => varray[
       b"binary",
       (string)"binary"],

       // object data
/*8*/
'object' => varray[
      new classA()]
];

// loop through each element of $inputs to check the behavior of array_diff_assoc
$iterator = 1;
foreach($inputs as $key => $input) {
  echo "\n-- Iteration $iterator --\n";
  var_dump( array_diff_assoc($input, $array));
  $iterator++;
};
echo "Done";
}
