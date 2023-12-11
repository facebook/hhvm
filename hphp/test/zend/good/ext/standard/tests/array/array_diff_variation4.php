<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff() compares indexed arrays containing different
 * data types as values in place of $arr2
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$array = vec[1, 2];

//get heredoc
$heredoc = <<<END
This is a heredoc
END;

//array of values to iterate over
$values = dict[

/*1*/"empty array" => vec[],

/*2*/
"int" => vec[
      // int data
      0,
      1,
      12345,
      -2345],

/*3*/
"float" => vec[
      // float data
       10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5],

/*4*/
"null" => vec[
      // null data
      NULL,
      null],

/*5*/
"boolean" => vec[
      // boolean data
      true,
      false,
      TRUE,
      FALSE],

/*6*/
"empty" => vec[
      // empty data
      "",
      ''],

/*7*/
"string" => vec[
      // string data
      "string",
      'string',
      $heredoc],

/*8*/
"binary" => vec[
       // binary data
       b"binary",
       (string)"binary"]
];

// loop through each element of the array for $arr2
$iterator = 1;
foreach($values as $value) {
      echo "\n Iteration: $iterator\n";
      var_dump( array_diff($array, $value) );
      $iterator++;
};

echo "Done";
}
