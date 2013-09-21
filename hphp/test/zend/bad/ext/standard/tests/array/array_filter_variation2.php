<?php
/* Prototype  : array array_filter(array $input [, callback $callback])
 * Description: Filters elements from the array via the callback. 
 * Source code: ext/standard/array.c
*/

/* Passing different scalar and nonscalar values in place of 'callback' argument
*/
echo "*** Testing array_filter() : usage variations - unexpected values for 'callback' function***\n";

// Initialise variables
$input = array('value1', 'value2', 'value3', 'value4');

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// class definition for object variable
class MyClass
{
  public function __toString()
  {
    return 'object';
  }
}

// resource variable
$fp = fopen(__FILE__, 'r');

// different scalar and nonscalar values in place of callback function
$values = array(

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

        // array data
/*10*/  array(),
        array(0),
        array(1),
        array(1, 2),
        array('color' => 'red', 'item' => 'pen'),

        // null data
/*15*/  NULL,
        null,

        // boolean data
/*17*/  true,
        false,
        TRUE,
        FALSE,

        // empty data
/*21*/  "",
        '',

        // string data
/*23*/  "string",
        'string',

        // object data
/*25*/  new MyClass(),

        // resource data
        $fp,

        // undefined data
        @$undefined_var,

        // unset data
/*28*/  @$unset_var,
);

// loop through each element of the 'values' for callback
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count + 1)." --";
  var_dump( array_filter($input, $values[$count]) );
};

// closing resource
fclose($fp);

echo "Done"
?>