<?php
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback. 
 * Source code: ext/standard/array.c
*/

/* Passing different scalar and nonscalar values for 'input' argument
*/
echo "*** Testing array_filter() : usage variations - unexpected values for 'input'***\n";

/* Callback function
 * Prototype : bool always_true(array $input)
 * Parameters : array for which each elements needs to be used in function
 * Return value : Returns true for each element
 * Discription : function applied to each element of the passed array and returns true
 */
function always_true($input)
{
  return true;
}


// get an unset variable
$unset_var = 10;
unset ($unset_var);

// class definition for object variable
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

// resource variable
$fp = fopen(__FILE__, 'r');

// different values for 'input' argument
$input_values = array(

        // int data
/*1*/   0,
        1,
        12345,
        -2345,

        // float data
/*5*/   10.5,
        -10.5,
        12.3456789000e10,
        12.3456789000E-10,
        .5,


        // null data
/*10*/  NULL,
        null,

        // boolean data
/*12*/  true,
        false,
        TRUE,
        FALSE,

        // empty data
/*16*/  "",
        '',

        // string data
/*18*/  "string",
        'string',

        // object data
/*20*/  new MyClass(),
 
        // resource data
        $fp,

        // undefined data
        @$undefined_var,

        // unset data
/*23*/  @$unset_var,
);

// loop through each element of the array for input
for($count = 0; $count < count($input_values); $count++) {
  echo "-- Iteration ".($count + 1)." --\n";
  var_dump( array_filter($input_values[$count],"always_true") );
};

// closing resource
fclose($fp);

echo "Done"
?>