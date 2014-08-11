<?php
/* Prototype  : array token_get_all(string $source)
 * Description: splits the given source into an array of PHP languange tokens 
 * Source code: ext/tokenizer/tokenizer.c
*/

/*
 * Passing different scalar/non-scalar values in place of 'source' argument
 *   It returns either T_INLINE_HTML by converting values into string or gives warning
*/

echo "*** Testing token_get_all() : unexpected values for 'source' argument ***\n";

// get an unset variable
$unset_var = 10;
unset ($unset_var);

// class definition
class MyClass
{
  public function __toString()
  {
    return "object";
  }
}

// get resource
$fp = fopen(__FILE__, 'r');

// different scalar/nonscalar values for 'source'
$source_values = array(

       // int data
/*1*/  0,
       1,
       12345,
       -2345,

       // float data
/*5*/  10.5,
       -10.5,
       10.1234567e8,
       10.7654321E-8,
       .5,

       // array data
/*10*/ array(),
       array(0),
       array(1),
       array(1, 2),
       array('color' => 'red', 'item' => 'pen'),

       // null data
/*15*/ NULL,
       null,

       // boolean data
/*17*/ true,
       false,
       TRUE,
       FALSE,

       // empty string
/*21*/ "",
       '',

       // object data
/*23*/ new MyClass(),
 
       // resource data
       $fp,

       // undefined data
       @$undefined_var,

       // unset data
/*26*/ @$unset_var,
);

for($count = 0; $count < count($source_values); $count++) {
  echo "--Iteration ".($count + 1)." --\n";
  var_dump( token_get_all($source_values[$count]));
};

fclose($fp);
echo "Done"
?>
