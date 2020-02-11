<?hh
/* Prototype  : array array_diff(array $arr1, array $arr2 [, array ...])
 * Description: Returns the entries of $arr1 that have values which are not
 * present in any of the others arguments.
 * Source code: ext/standard/array.c
 */

/*
 * Test how array_diff() compares indexed arrays containing different
 * data types as values in place of $arr1
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing array_diff() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$array = varray[1, 2];

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//get heredoc
$heredoc = <<<END
This is a heredoc
END;

//array of values to iterate over
$values = darray[

/*1*/"empty array" => varray[],

/*2*/
"int" => varray[
      // int data
      0,
      1,
      12345,
      -2345],

/*3*/
"float" => varray[
      // float data
       10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5],

/*4*/
"null" => varray[
      // null data
      NULL,
      null],

/*5*/
"boolean" => varray[
      // boolean data
      true,
      false,
      TRUE,
      FALSE],

/*6*/
"empty" => varray[
      // empty data
      "",
      ''],

/*7*/
"string" => varray[
      // string data
      "string",
      'string',
      $heredoc],

/*8*/
"binary" => varray[
       // binary data
       b"binary",
       (string)"binary"],

/*9*/
"undefined" => varray[
      // undefined data
      @$undefined_var],

/*10*/
"unset" => varray[
      // unset data
      @$unset_var]
];

// loop through each element of the array for arr1
$iterator = 1;
foreach($values as $value) {
      echo "\n Iteration: $iterator \n";
      var_dump( array_diff($value, $array) );
      $iterator++;
};

echo "Done";
}
